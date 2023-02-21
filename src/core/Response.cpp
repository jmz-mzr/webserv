#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>

#include <cerrno>
#include <cstring>
#include <cctype>
#include <cstdlib>

#include <map>
#include <sstream>
#include <limits>

#include "core/Response.hpp"
#include "webserv_config.hpp"
#include "utils/global_defs.hpp"
#include "utils/Logger.hpp"
#include "utils/utils.hpp"

#define INTERNAL_REDIRECT	-1

#define REWRITE_LOOP1		"rewrite or internal redirection cycle"
#define REWRITE_LOOP2		" while internally redirecting to"
#define REWRITE_LOOP		REWRITE_LOOP1 REWRITE_LOOP2

namespace	webserv
{

	/**************************************************************************/
	/*                       CONSTRUCTORS / DESTRUCTORS                       */
	/**************************************************************************/

	Response::Response(): _requestedFilename(XSTR(WEBSERV_ROOT)),
//							_requestedFileFd(-1),
							_fileBuffer(0),
							_fileBufferSize(0),
							_bufferPos(0),
							_indexDirectory(0),
							_responseCode(0),
							_contentType("application/octet-stream"),
							_contentLength(-1),
							_lastModifiedTime(-1),
							_isKeepAlive(true),
							_isChunkedResponse(false),
							_isFileResponse(false),
							_isResponseReady(false)
	{
		LOG_INFO("New Response instance");
	}

	Response::Response(const Response& src):
									_requestedFilename(XSTR(WEBSERV_ROOT)),
//									_requestedFileFd(src._requestedFileFd),
									_fileBuffer(0),
									_fileBufferSize(0),
									_bufferPos(0),
									_indexDirectory(0),
									_responseCode(0),
									_contentType("application/octet-stream"),
									_contentLength(-1),
									_lastModifiedTime(-1),
									_isKeepAlive(true),
									_isChunkedResponse(false),
									_isFileResponse(false),
									_isResponseReady(false)
	{
		// NOTE: Except at Client creation (inserted in the client list),
		// the Response should not be copied

		(void)src;
		LOG_INFO("Response copied");
	}

	Response::~Response()
	{
//		if (_requestedFileFd >= 0)
		if (_requestedFile.is_open())
			_closeRequestedFile();
		if (_fileBuffer != 0)
			delete[] _fileBuffer;
		if (_indexDirectory != 0)
			_closeIndexDirectory();
	}

	/**************************************************************************/
	/*                            MEMBER FUNCTIONS                            */
	/**************************************************************************/

	const std::string&	Response::getResponseBuffer() const
	{
		return (_responseBuffer);
	}

/*	void	Response::setResponseCode(int responseCode)
	{
		_responseCode = responseCode;
	}
*/
	bool	Response::isKeepAlive() const
	{
		if (!_isKeepAlive)
			return (false);
		if (_responseCode == 400 || _responseCode == 408 || _responseCode == 413
				|| _responseCode == 414 || _responseCode == 495
				|| _responseCode == 496 || _responseCode == 497
				|| _responseCode == 500 || _responseCode == 501)
			return (false);
		return (true);
	}

	bool	Response::isPartialResponse() const
	{
		if (_isChunkedResponse || _isFileResponse)
			return (true);
		return (false);
	}

	std::string	Response::_getETag(const struct stat* fileInfos) const
	{
		time_t					lastModified;
		int64_t					size;
		std::ostringstream		eTag;

		if (fileInfos) {
			lastModified = fileInfos->st_mtime;
			size = fileInfos->st_size;
		} else {
			lastModified = _lastModifiedTime;
			size = _contentLength;
		}
		if (lastModified < 0 || size < 0)
			return ("");
		eTag << std::hex << "\"";
		if (static_cast<int64_t>(lastModified) < 0)
			eTag << "-" << static_cast<uint64_t>(-lastModified);
		else
			eTag << static_cast<uint64_t>(lastModified);
		eTag << "-";
		if (size < 0)
			eTag << "-" << static_cast<uint64_t>(-size);
		else
			eTag << static_cast<uint64_t>(size);
		eTag << "\"";
		return (eTag.str());
	}

	std::string	Response::_getAllowedMethods(const Request& request) const
	{
		typedef Location::limit_except_set	limit_set;

		const limit_set&			allowedMethods = request.getLocation()->
															getLimitExcept();
		limit_set::const_iterator	it = allowedMethods.begin();
		std::ostringstream			allowStr;

		allowStr << std::uppercase;
		if (!allowedMethods.empty()) {
			while (it != allowedMethods.end()) {
				allowStr << *it;
				if (++it != allowedMethods.end())
					allowStr << ", ";
			}
		} else
			allowStr << "GET, HEAD, POST, DELETE";
		return (allowStr.str());
	}

	void	Response::_loadHeaders(const Request& request)
	{
		const char*			connection = _isKeepAlive ? "keep-alive" : "close";
		std::ostringstream	headers;

		headers << "HTTP/1.1 " << _responseCode << " "
			<< Response::_getResponseStatus(_responseCode) << CRLF
			<< "Server: webserv" << CRLF
			<< "Date: " << Response::_getDate() << CRLF
			<< "Content-Type: " << _contentType << CRLF;	// only if content?
		if (_responseCode == 405)
			headers << "Allow: " << _getAllowedMethods(request) << CRLF;
		if (_contentLength != -1 && _responseCode > 199 && _responseCode != 204)
			headers << "Content-Length: " << _contentLength << CRLF;
		if (_lastModifiedTime != -1)
			headers << "Last-Modified: " << _getDate(_lastModifiedTime) << CRLF;
		if (!_location.empty())
			headers << "Location: " << _location << CRLF;
		if (_isChunkedResponse)
			headers << "Transfer-Encoding: chunked" << CRLF;
		headers << "Connection: " << connection << CRLF;
		if (_lastModifiedTime != -1)
			headers << "ETag: " << _getETag() << CRLF;
		headers << CRLF;
		_responseBuffer = headers.str();
		LOG_DEBUG("HTTP Headers:\n" << _responseBuffer);
	}

	void	Response::_logError(const Request& request,
								const char* errorAt,
								const char* errorType,
								const char* filename) const try
	{
		std::ostringstream	debugInfos;
		std::string			space1((errorAt[0] != '\0'), ' ');
		std::string			space2((errorType[0] != '\0'), ' ');

		debugInfos << "client: " << request.getClientSocket().getIpAddr()
			<< ", server: " << request.getServerName()
			<< ", request: \"" << request.getRequestLine()
			<< "\", host: \"" << request.getHost() << "\"";
		if (!filename)
			filename = _requestedFilename.c_str();
		if (errorAt[0] != '\0' && errorType[0] != '\0') {
			LOG_ERROR(errorAt << space1 << "\"" << filename << "\" "
					<< errorType << " (" << errno << ": " << strerror(errno)
					<< "), " << debugInfos.str());
		} else
			LOG_ERROR(errorAt << space1 << "\"" << filename << "\""
					<< space2 << errorType << ", " << debugInfos.str());
	}
	catch (const std::exception& e) {
		LOG_ERROR("Logging error: " << e.what());
	}

	int	Response::_loadDirEntry(const Request& request, const char* entryName)
	{
		struct stat		fileInfos;

		LOG_DEBUG("HTTP autoindex file: " << entryName);
		if (entryName[0] == '.')
			return (0);
		memset(&fileInfos, 0, sizeof(fileInfos));
		if (stat(_requestedFilename.c_str(), &fileInfos) < 0) {
			if (errno != ENOENT && errno != ELOOP) {
				_logError(request, "stat()", "failed");
				if (errno == EACCES)
					return (0);
				return (500);
			}
			if (lstat(_requestedFilename.c_str(), &fileInfos) < 0) {
				_logError(request, "lstat()", "failed");
				return (500);
			}
		}
		_dirEntrySet.insert(std::make_pair(entryName, fileInfos));
		return (0);
	}

	void	Response::_closeIndexDirectory()
	{
		if (closedir(_indexDirectory) < 0)
			LOG_ERROR("Bad closedir() on \"" << _requestedFilename << "\"");
		_indexDirectory = 0;
	}

	int	Response::_loadDirEntries(const Request& request)
	{
		struct dirent*	dirEntry;
		size_t			pathLen = _requestedFilename.size();
		int				errorCode;

		while (1) {
			errno = 0;
			dirEntry = readdir(_indexDirectory);
			if (!dirEntry) {
				if (errno != 0) {
					_logError(request, "readdir()", "failed");
					return (500);
				}
				break ;
			}
			_requestedFilename += '/';
			_requestedFilename += dirEntry->d_name;
			errorCode = _loadDirEntry(request, dirEntry->d_name);
			_requestedFilename[pathLen] = '\0';
			if (errorCode != 0)
				return (errorCode);
		}
		_closeIndexDirectory();
		return (0);
	}

	std::string	Response::_escapeHtml(const std::string& str,
										size_t maxLen) const
	{
		std::string					strEscaped;
		size_t						pos;

		for (std::string::const_iterator strIt = str.begin();
				strIt != str.end() && strEscaped.size() <= maxLen; ++strIt) {
			if (*strIt == '&')
				strEscaped += "&amp;";
			else if (*strIt == '<')
				strEscaped += "&lt;";
			else if (*strIt == '>')
				strEscaped += "&gt;";
			else if (*strIt == '"')
				strEscaped += "&quot;";
			else
				strEscaped += *strIt;
		}
		if (strEscaped.size() > maxLen) {
			pos = (maxLen >= 3) ? maxLen - 3 : 0;
			strEscaped.replace(pos, std::string::npos, "..&gt;");
		}
		return (strEscaped);
	}

	std::string	Response::_escapeUriComponent(const std::string& uri) const
	{
		static char		hex[] = "0123456789ABCDEF";
		std::string		uriEscaped;
		char			c;

		for (std::string::const_iterator uriIt = uri.begin();
				uriIt != uri.end(); ++uriIt) {
			c = *uriIt;
			if (std::isalnum(c) || c == '-' || c == '.' || c == '_' || c == '~')
				uriEscaped += c;
			else {
				uriEscaped += '%';
				uriEscaped += hex[static_cast<unsigned char>(c) >> 4];
				uriEscaped += hex[static_cast<unsigned char>(c) & 0x0f];
			}
		}
		return (uriEscaped);
	}

	std::string	Response::_escapeUri(const std::string& uri) const
	{
		static char		hex[] = "0123456789ABCDEF";
		std::string		uriEscaped;
		char			c;

		for (std::string::const_iterator uriIt = uri.begin();
				uriIt != uri.end(); ++uriIt) {
			c = *uriIt;
			if (!std::isprint(c) || c == ' ' || c == '"' || c == '<' || c == '>'
					|| c == '\\' || c == '^' || c == '`' || c == '{' || c == '}'
					|| c == '|' || c == '#' || c == '%' || c == '?') {
				uriEscaped += '%';
				uriEscaped += hex[static_cast<unsigned char>(c) >> 4];
				uriEscaped += hex[static_cast<unsigned char>(c) & 0x0f];
			} else
				uriEscaped += c;
		}
		return (uriEscaped);
	}

	std::string	Response::_getFileDate(const struct stat& fileInfos) const
	{
		char			buffer[32];
		struct tm*		gmtTime = std::gmtime(&fileInfos.st_mtime);

		std::strftime(buffer, 32, "%d-%b-%Y %H:%M ", gmtTime);
		return (buffer);
	}

	std::string	Response::_getFileSize(const struct stat& fileInfos) const
	{
		std::ostringstream	fileSize;
		std::string			buffer(19, ' ');

		if (S_ISDIR(fileInfos.st_mode)) {
			buffer[18] = '-';
			return (buffer);
		}
		fileSize << fileInfos.st_size;
		buffer = std::string(19 - fileSize.str().size(), ' ');
		buffer += fileSize.str();
		return (buffer);
	}

	void	Response::_loadAutoindexEntry(const _dir_entry_pair& entry)
	{
		std::string		escapedUri;

		_responseBuffer += "<a href=\"";
		_responseBuffer += _escapeUriComponent(entry.first);
		if (S_ISDIR(entry.second.st_mode))
			_responseBuffer += '/';
		_responseBuffer += "\">";
		escapedUri = _escapeHtml(entry.first, AUTOINDEX_NAME_LEN);
		if (S_ISDIR(entry.second.st_mode)
				&& escapedUri.size() < AUTOINDEX_NAME_LEN)
			escapedUri += '/';
		_responseBuffer += escapedUri;
		_responseBuffer += "</a>";
		_responseBuffer += std::string(AUTOINDEX_NAME_LEN > escapedUri.size()
				? AUTOINDEX_NAME_LEN - escapedUri.size() + 1 : 1, ' ');
		_responseBuffer += _getFileDate(entry.second);
		_responseBuffer += _getFileSize(entry.second);
		_responseBuffer += CRLF;
	}

	void	Response::_loadChunkHeaderAndTrailer(bool isLastChunk)
	{
		std::ostringstream	chunkSize;

		chunkSize << _responseBuffer.size();
		_responseBuffer.insert(0, chunkSize.str());
		if (isLastChunk)
			_responseBuffer += CRLF "0" CRLF CRLF;
	}

	void	Response::_loadAutoindexHtml(const Request& request)
	{
		std::string		escapedUri = _escapeHtml(request.getUri());

		_responseBuffer += "<html>" CRLF "<head><title>Index of ";
		_responseBuffer += escapedUri;
		_responseBuffer += "</title></head>" CRLF "<body>" CRLF "<h1>Index of ";
		_responseBuffer += escapedUri;
		_responseBuffer += "</h1><hr><pre><a href=\"../\">../</a>" CRLF;
		for (_dir_entry_set::iterator entry = _dirEntrySet.begin();
				entry != _dirEntrySet.end(); ++entry) {
			_loadAutoindexEntry(*entry);
		}
		_responseBuffer += "</pre><hr></body>" CRLF "</html>" CRLF;
		_loadChunkHeaderAndTrailer(true);
	}

	void	Response::_prepareChunkedResponse(Request& request)
	{
		int		errorCode;

		_clearBuffer();
		if (_indexDirectory != 0) {
			errorCode = _loadDirEntries(request);
			if (errorCode != 0)
				return (prepareErrorResponse(request, errorCode));
			_loadAutoindexHtml(request);
			_isChunkedResponse = false;
			_isResponseReady = true;
			return ;
		}
		LOG_DEBUG("No index directory to load chunked response");
		return (prepareErrorResponse(request, 500));
	}

	void	Response::_clearBuffer()
	{
		_responseBuffer.clear();
		if (_fileBuffer) {
			delete[] _fileBuffer;
			_fileBuffer = 0;
		}
		_fileBufferSize = 0;
		_bufferPos = 0;
	}

	bool	Response::sendResponse(const Request& request, int clientFd,
									int ioFlags)
	{
		const char*	buff = _fileBuffer ? _fileBuffer : _responseBuffer.data();
		size_t		size = _fileBuffer ? _fileBufferSize
									: _responseBuffer.size();
		size_t		toSend = size - _bufferPos;
		ssize_t		sent;

		if (toSend > SEND_BUFFER_SIZE) {
			LOG_DEBUG("Limit send to first " << SEND_BUFFER_SIZE << " bytes");
			toSend = SEND_BUFFER_SIZE;
		}
		sent = send(clientFd, buff + _bufferPos, toSend, ioFlags);
		LOG_DEBUG("" << sent << " bytes were sent");
		if (sent == -1 || (sent == 0 && toSend != 0)) {
			_logError(request, "Error while sending", "to client");
			return (false);
		}
		_bufferPos += static_cast<size_t>(sent);
		if (_bufferPos == size) {
			_clearBuffer();
			_isResponseReady = false;
		}
		return (true);
	}

	void	Response::_prepareFileResponse(Request& request)
	{
		// TO DO: read while (size < SEND_BUF) && no exception raised?

		if (_fileBuffer != 0 && (_bufferPos != _fileBufferSize)) {
			_isResponseReady = true;
			return ;
		}
		if (_fileBuffer != 0)
			delete[] _fileBuffer;
		if (_contentLength > READ_BUFFER_SIZE)
			_fileBufferSize = READ_BUFFER_SIZE;
		else
			_fileBufferSize = static_cast<size_t>(_contentLength);
		_fileBuffer = new char[_fileBufferSize];
		_requestedFile.read(_fileBuffer, static_cast<ssize_t>(_fileBufferSize));
		LOG_DEBUG("Read " << _requestedFile.gcount() << " bytes");
		if (_requestedFile.bad() || (_requestedFile.fail()
					&& !_requestedFile.eof() && _requestedFile.gcount() <= 0)) {
			_logError(request, "Error while reading", "for response");
			return (prepareErrorResponse(request, 500));
		}
		_fileBufferSize = static_cast<size_t>(_requestedFile.gcount());
		_contentLength -= static_cast<int64_t>(_fileBufferSize);
		if (_contentLength == 0)
			_isFileResponse = false;
		_isResponseReady = true;
	}

	int	Response::_loadInternalRedirect(Request& request,
										const std::string& redirectTo)
	{
		// TO DO: 1) Return error only after 10 identical redirect (like NGINX)?
		// 2) Do we need to first clear the response (if return/error_page)?

		int		errorCode;

		if (request.isInternalRedirect() && redirectTo == request.getUri()) {
			_logError(request, REWRITE_LOOP, "");
			return (500);
		}
		errorCode = request.loadInternalRedirect(redirectTo);
		if (errorCode)
			return (errorCode);
		prepareResponse(request);
		return (INTERNAL_REDIRECT);
	}

	int	Response::_loadAutoIndex(const Request& request)
	{
		if ((request.getRequestMethod() != "GET"
				&& request.getRequestMethod() != "HEAD")
				|| !request.getLocation()->isAutoIndex()) {
			_logError(request, "", "directory index is forbidden");
			return (403);
		}
		LOG_DEBUG("HTTP autoindex: \"" << _requestedFilename << "\"");
		_indexDirectory = opendir(_requestedFilename.c_str());
		if (!_indexDirectory) {
			_logError(request, "opendir()", "failed");
			if (errno == ENOENT || errno == ENOTDIR || errno == ENAMETOOLONG)
				return (404);
			if (errno == EACCES)
				return (403);
			return (500);
		}
		if (_responseCode == 0)
			_responseCode = 200;
		_contentType = "text/html";
		_isChunkedResponse = true;
		return (0);
	}

	int	Response::_loadDirIndex(const Request& request, size_t indexLen)
	{
		size_t			dirPos = _requestedFilename.size() - indexLen - 1;
		char			dirChar = _requestedFilename[dirPos + (dirPos == 0)];
		struct stat		dirInfos;

		dirPos += (dirPos == 0);
		_requestedFilename[dirPos] = '\0';
		LOG_DEBUG("HTTP index check dir: \"" << _requestedFilename << "\"");
		memset(&dirInfos, 0, sizeof(dirInfos));
		if (stat(_requestedFilename.c_str(), &dirInfos) < 0) {
			if (errno != EACCES && errno != ENOENT) {
				_logError(request, "stat()", "failed");
				return (500);
			}
			if (errno == EACCES)
				return (_loadAutoIndex(request));
			_requestedFilename[dirPos] = dirChar;
			_logError(request, "", "is not found");
			return (404);
		}
		if (S_ISDIR(dirInfos.st_mode))
			return (_loadAutoIndex(request));
		_requestedFilename[dirPos] = dirChar;
		_logError(request, "", "is not a directory");
		return (500);
	}

	int	Response::_loadIndex(Request& request)
	{
		const std::string&	index = request.getLocation()->getIndex();
		struct stat			fileInfos;

		if (index[0] == '/')
			return (_loadInternalRedirect(request, index));
		_requestedFilename += index;
		LOG_DEBUG("Open index \"" << _requestedFilename << "\"");
		memset(&fileInfos, 0, sizeof(fileInfos));
		if (stat(_requestedFilename.c_str(), &fileInfos) < 0) {
			LOG_DEBUG("stat() \"" << _requestedFilename << "\" failed ("
					<< errno << ": " << strerror(errno));
			if (errno == EACCES) {
				_logError(request, "", "is forbidden");
				return (403);
			} else if (errno == ENOTDIR || errno == ENAMETOOLONG) {
				_logError(request, "", "is not found");
				return (404);
			} else if (errno != ENOENT) {
				_logError(request, "stat()", "failed");
				return (500);
			}
			return (_loadDirIndex(request, index.size()));
		}
		return (_loadInternalRedirect(request, _requestedFilename));
	}

	void	Response::_closeRequestedFile()
	{
//		if (close(_requestedFileFd) < 0)
//			LOG_ERROR("Bad close() on fd=" << _requestedFileFd);
//		_requestedFileFd = -1;
		_requestedFile.clear();
		_requestedFile.close();
		if (_requestedFile.fail()) {
			if (!_tmpCgiBodyFilename.empty()) {
				LOG_ERROR("Bad close() on \"" << _tmpCgiBodyFilename << "\"");
			} else
				LOG_ERROR("Bad close() on \"" << _requestedFilename << "\"");
			_requestedFile.clear();
		}
	}

	bool	Response::_openAndStatFile(const Request& request,
										struct stat* fileInfos)
	{
		LOG_DEBUG("HTTP filename: \"" << _requestedFilename << "\"");
//		_requestedFileFd = open(_requestedFilename.c_str(),
//				O_RDONLY | O_NONBLOCK);
		_requestedFile.open(_requestedFilename.c_str(), std::ios::in
				| std::ios::binary);
//		if (_requestedFileFd < 0) {
		if (_requestedFile.fail()) {
			_logError(request, "open()", "failed");
			return (false);
		}
		memset(fileInfos, 0, sizeof(*fileInfos));
		if (stat(_requestedFilename.c_str(), fileInfos) < 0) {
			_logError(request, "stat()", "failed");
			//_closeRequestedFile();
			return (false);
		} else if (S_ISDIR(fileInfos->st_mode))
			_closeRequestedFile();
		else
			LOG_DEBUG("HTTP file opened: " << _requestedFilename);
//			LOG_DEBUG("HTTP file fd: " << _requestedFileFd);
		return (true);
	}

	void	Response::_loadDirLocation(const Request& request)
	{
		_loadRelativeLocationPrefix(request);
		_location += _escapeUri(request.getUri());
		if (!request.getQuery().empty())
			_location += std::string("?") + request.getQuery();
	}

	std::string	Response::_getFileExtension()
	{
		const char*		name = _requestedFilename.c_str();
		int				i = static_cast<int>(_requestedFilename.size()) - 1;
		int				ext = 0;

		if (i <= 1 || !isprint(name[i]) || name[i] == '/' || name[i] == '.')
			return ("");
		while (--i > 0) {
			if (!isprint(name[i]) || name[i] == '/')
				return ("");
			if (_requestedFilename[i] == '.') {
				for (int j = i - 1; j >= 0; --j) {
					if (!isprint(name[j]) || name[j] == '/')
						return ("");
					if (isprint(name[j]) && name[j] != '.') {
						ext = i + 1;
						break ;
					}
				}
				break ;
			}
		}
		if (ext != 0)
			return (&name[ext]);
		return ("");
	}

	void	Response::_loadFileHeaders(const struct stat* fileInfos)
	{
		if (_responseCode == 0)
			_responseCode = 200;
		_contentLength = fileInfos->st_size;
		_lastModifiedTime = fileInfos->st_mtime;
		_contentType = _getContentType(_getFileExtension());
	}

	bool	Response::_findMatch(const std::string& value,
									const std::string& eTag) const
	{
		std::string::const_iterator			c;

		if (eTag.empty())
			return (false);
		static_cast<void>(value.c_str());
		c = value.begin();
		while (c != value.end()) {
			if (eTag.size() > static_cast<size_t>(value.end() - c))
				return (false);
			if (eTag.find(&(*c), 0, eTag.size()) == 0) {
				c += eTag.size();
				while (*c == ' ' || *c == '\t')
					++c;
				if (c == value.end() || *c == ',')
					return (true);
			}
			while (c != value.end() && *c != ',')
				++c;
			while (c != value.end()) {
				if (*c != ' ' && *c != '\t' && *c != ',')
					break ;
				++c;
			}
		}
		return (false);
	}

	int	Response::_checkIfMatch(const Request& request,
								const struct stat* fileInfos,
								bool ifNoneMatch) const
	{
		Request::header_map					headers = request.getHeaders();
		std::string							name = ifNoneMatch ? "If-None-Match"
																: "If-Match";
		Request::header_map::const_iterator	it = headers.find(name);
		std::string							value;
		const std::string&					method = request.getRequestMethod();

		if (it == headers.end())
			return (0);
		value = it->second;
		LOG_DEBUG("HTTP " << name << ": \"" << value
				<< "\" (ETag: \"" << _getETag(fileInfos) << "\")");
		if (ifNoneMatch) {
			if ((fileInfos->st_mode > 0 && value.size() == 1 && value == "*")
					|| _findMatch(value, _getETag(fileInfos))) {
				if (method == "GET" || method == "HEAD")
					return (304);
				return (412);
			}
			return (0);
		}
		if ((fileInfos->st_mode > 0 && value.size() == 1 && value == "*")
				|| _findMatch(value, _getETag(fileInfos)))
			return (0);
		return (412);
	}

	time_t	Response::_parseTime(const char* timeStr) const
	{
		struct tm	time;
		char*		converted;

		memset(&time, 0, sizeof(time));
		converted = strptime(timeStr, "%a, %d %b %Y %H:%M:%S GMT", &time);
		if (!converted || *converted != '\0') {
			memset(&time, 0, sizeof(time));
			converted = strptime(timeStr, "%a, %d-%b-%y %H:%M:%S GMT", &time);
		}
		if (!converted || *converted != '\0') {
			memset(&time, 0, sizeof(time));
			converted = strptime(timeStr, "%a %b %d %H:%M:%S %Y", &time);
		}
		if (!converted || *converted != '\0')
			return (-1);
		return (std::mktime(&time));
	}

	int	Response::_checkIfModifiedSince(const Request& request,
										const struct stat* fileInfos,
										bool ifUnmodifiedSince) const
	{
		const std::string&					method = request.getRequestMethod();
		Request::header_map					headers = request.getHeaders();
		std::string							name = ifUnmodifiedSince ?
													"If-Unmodified-Since"
													: "If-Modified-Since";
		std::string							match = ifUnmodifiedSince ?
													"If-Match"
													: "If-None-Match";
		Request::header_map::const_iterator	it = headers.find(name);
		time_t								time;

		if ((!ifUnmodifiedSince && method != "GET" && method != "HEAD")
				|| it == headers.end() || headers.find(match) != headers.end())
			return (0);
		time = _parseTime(it->second.c_str());
		LOG_DEBUG("HTTP " << name << ": \"" << it->second << "\" [=" << time
				<< "] (Last-Modified: " << fileInfos->st_mtime << ")");
		if (ifUnmodifiedSince) {
			if (fileInfos->st_mode && time >= 0 && time < fileInfos->st_mtime)
				return (412);
			return (0);
		}
		if (time < 0 || time < fileInfos->st_mtime)
			return (0);
		return (304);
	}

	int	Response::_checkConditionalHeaders(const Request& request,
											const struct stat* fileInfos) const
	{
		int		responseCode;

		responseCode = _checkIfMatch(request, fileInfos);
		if (responseCode)
			return (responseCode);
		responseCode = _checkIfModifiedSince(request, fileInfos, true);
		if (responseCode)
			return (responseCode);
		responseCode = _checkIfMatch(request, fileInfos, true);
		if (responseCode)
			return (responseCode);
		responseCode = _checkIfModifiedSince(request, fileInfos);
		if (responseCode)
			return (responseCode);
		return (0);
	}

	int	Response::_openRequestedFile(const Request& request)
	{
		// TO DO: Make sure the requested file search is case-insensitive

		struct stat		fileInfos;

		if (!_openAndStatFile(request, &fileInfos)) {
			if (errno == EACCES)
				return (403);
			if (errno == ENOENT || errno == ENOTDIR || errno == ENAMETOOLONG)
				return (404);
			return (500);
		} else if (S_ISDIR(fileInfos.st_mode)) {
			LOG_DEBUG("HTTP dir");
			_loadDirLocation(request);
			return (301);
		} else if (!S_ISREG(fileInfos.st_mode)) {
			_logError(request, "", "is not a regular file");
			return (404);
		} else if (request.getRequestMethod() == "POST") {
			return (405);
		}
		_loadFileHeaders(&fileInfos);
		if (_contentLength > 0)
			_isFileResponse = true;
		return (_checkConditionalHeaders(request, &fileInfos));
	}

	void	Response::_deleteDirectory(const Request& request,
										const char* dirname)
	{
		LOG_DEBUG("HTTP delete dir: \"" << dirname << "\"");
		if (rmdir(dirname) < 0)
			_logError(request, "rmdir()", "failed", dirname);
	}

	void	Response::_deleteFile(const Request& request,
									const char* filename)
	{
		LOG_DEBUG("HTTP delete file: \"" << filename << "\"");
		if (unlink(filename) < 0)
			_logError(request, "unlink()", "failed", filename);
	}

	bool	Response::_removeDirEntry(const Request& request,
										const char* dirPath,
										const char* entryName,
										bool* hasError) try
	{
		std::string		filepath;
		struct stat		fileInfos;

		LOG_DEBUG("Tree element name: \"" << entryName << "\"");
		if (std::string(entryName) == "." || std::string(entryName) == "..")
			return (true);
		filepath = std::string(dirPath) + "/" + entryName;
		LOG_DEBUG("Tree element path: \"" << filepath << "\"");
		memset(&fileInfos, 0, sizeof(fileInfos));
		if (stat(filepath.c_str(), &fileInfos) < 0) {
			_logError(request, "stat()", "failed", filepath.c_str());
			return (true);
		} else if (!S_ISDIR(fileInfos.st_mode)) {
			if (S_ISREG(fileInfos.st_mode)) {
				LOG_DEBUG("Tree file: \"" << filepath << "\"");
			} else
				LOG_DEBUG("Tree special: \"" << filepath << "\"");
			_deleteFile(request, filepath.c_str());
		} else {
			LOG_DEBUG("Tree directory: \"" << filepath << "\"");
			if (!_removeDirectoryTree(request, filepath.c_str()))
				return (false);
			_deleteDirectory(request, filepath.c_str());
		}
		return (true);
	}
	catch (const std::exception& e) {
		LOG_ERROR("Error while removing directory entry: " << e.what());
		*hasError = true;
		return (false);
	}

	bool	Response::_removeDirectoryTree(const Request& request,
											const char* dirPath)
	{
		DIR*			directory;
		struct dirent*	dirEntry;
		bool			hasError = false;

		LOG_DEBUG("Traverse directory tree: \"" << dirPath << "\"");
		directory = opendir(dirPath);
		if (!directory) {
			_logError(request, "opendir()", "failed", dirPath);
			return (false);
		}
		while (1) {
			errno = 0;
			dirEntry = readdir(directory);
			if (!dirEntry && errno != 0) {
				_logError(request, "readdir()", "failed", dirPath);
				hasError = true;
			}
			if (!dirEntry)
				break ;
			if (!_removeDirEntry(request, dirPath, dirEntry->d_name, &hasError))
				break ;
		}
		if (closedir(directory) < 0)
			LOG_ERROR("Bad closedir() on \"" << dirPath << "\"");
		return (!hasError);
	}

	int	Response::_removeRequestedDirectory(const Request& request)
	{
		std::string		filepath = _requestedFilename;

		if (*request.getUri().rbegin() != '/') {
			_logError(request, "DELETE", "failed");
			return (409);
		}
		if (*filepath.rbegin() == '/')
			filepath.erase(filepath.end() - 1);
		if (!_removeDirectoryTree(request, filepath.c_str()))
			return (500);
		if (rmdir(_requestedFilename.c_str()) < 0) {
			_logError(request, "rmdir()", "failed");
			if (errno == ENOENT || errno == ENOTDIR || errno == ENAMETOOLONG)
				return (404);
			if (errno == EACCES || errno == EPERM)
				return (403);
			return (500);
		}
		_responseCode = 204;
		return (0);
	}

	int Response::_unlinkFile(const Request& request)
	{
		if (unlink(_requestedFilename.c_str()) < 0) {
			_logError(request, "unlink()", "failed");
			if (errno == ENOENT || errno == ENOTDIR || errno == ENAMETOOLONG)
				return (404);
			if (errno == EACCES || errno == EPERM || errno == EISDIR)
				return (403);
			return (500);
		}
		_responseCode = 204;
		return (0);
	}

	int	Response::_removeRequestedFile(const Request& request)
	{
		struct stat		fileInfos;
		int				responseCode;

		LOG_DEBUG("HTTP delete filename: \"" << _requestedFilename << "\"");
		memset(&fileInfos, 0, sizeof(fileInfos));
		if (lstat(_requestedFilename.c_str(), &fileInfos) < 0) {
			_logError(request, "lstat()", "failed");
			if (errno == ENOTDIR)
				return (409);
			if (errno == ENOENT || errno == ENAMETOOLONG)
				return (404);
			if (errno == EACCES)
				return (403);
			return (500);
		}
		responseCode = _checkConditionalHeaders(request, &fileInfos);
		if (responseCode)
			return (responseCode);
		if (S_ISDIR(fileInfos.st_mode))
			return (_removeRequestedDirectory(request));
		return (_unlinkFile(request));
	}

	void	Response::_setPostHeaders(const Request& request)
	{
		struct stat		fileInfos;

		_contentType = "text/html";
		_contentLength = _getPostResponseBody(request).size();
		memset(&fileInfos, 0, sizeof(fileInfos));
		if (stat(_requestedFilename.c_str(), &fileInfos) < 0)
			_logError(request, "stat()", "failed");
		else
			_lastModifiedTime = fileInfos.st_mtime;
		_loadRelativeLocationPrefix(request);
		_location += _escapeUri(request.getUri());
	}

	int	Response::_moveRequestTmpFile(const Request& request,
										const struct stat* fileInfos)
	{
		// TO DO: Add an option to create the full path if it doesn't exist ?

		int		responseCode = _checkConditionalHeaders(request, fileInfos);

		if (responseCode)
			return (responseCode);
		if (rename(request.getTmpFilename().c_str(),
					_requestedFilename.c_str()) < 0) {
			_logError(request, "rename()", "failed");
			return (500);
		}
		_responseCode = 201;
		_setPostHeaders(request);
		return (0);
	}

/*	int	Response::_appendRequestTmpFile(const Request& request)
	{
		request.getTmpFile().open(request.getTmpFilename().c_str(), std::ios::in
				| std::ios::binary);
		if (request.getTmpFile().fail()) {
			_logError(request, "open()", "failed",
					request.getTmpFilename().c_str());
			return (500);	// tmpFile deleted in clear request
		}
		_requestedFile.open(_requestedFilename.c_str(), std::ios::out
				| std::ios::app | std::ios::binary);
		if (_requestedFile.fail()) {
			_logError(request, "open()", "failed");
			return (500);	// tmpFile closed & deleted in clear request
		}
		_requestedFile << request.getTmpFile().rdbuf();
		if (_requestedFile.fail()) {
			_logError(request, "Error while appending request tmp file to", "");
			return (500);	// files closed & deleted in clear req/resp
		}
		request.closeTmpFile();
		_closeRequestedFile();
		return (0);
	}
*/
	int	Response::_handleRequestAlreadyExistingFile(const Request& request,
												const struct stat* fileInfos)
	{
		if (fileInfos->st_size == 0) {
			_loadRelativeLocationPrefix(request);
			_location += _escapeUri(request.getUri());
			return (303);
		}
		errno = EEXIST;
		_logError(request, "The file", "could not be created");
		return (409);
	}

	int	Response::_postRequestBody(const Request& request)
	{
		// TO DO: Before loading the request body, check in request after having
		// received the headers if the POST request will be processed ?
		// (i.e. there is no return directive, there is a CGI, or after the
		// root/alias translation, there is a filename that is not a directory,
		// and that doesn't already exist)
		// If so, just run this function with a dryRun parameter ?

		struct stat		fileInfos;

		if (*_requestedFilename.rbegin() == '/') {
			_logError(request, "Cannot POST to a directory:", "");
			return (409);
		}
		LOG_DEBUG("HTTP post filename: \"" << _requestedFilename << "\"");
		memset(&fileInfos, 0, sizeof(fileInfos));
		if (stat(_requestedFilename.c_str(), &fileInfos) < 0)
			return (_moveRequestTmpFile(request, &fileInfos));
		if (S_ISDIR(fileInfos.st_mode)) {
			errno = EISDIR;
			_logError(request, "The file", "could not be created");
			return (409);
		}
		memset(&fileInfos, 0, sizeof(fileInfos));
		if (stat(request.getTmpFilename().c_str(), &fileInfos) < 0) {
			_logError(request, "stat()", "failed");
			return (500);
		}
		return (_handleRequestAlreadyExistingFile(request, &fileInfos));
	}

	bool	Response::_loadFileWithAlias(Request& request)
	{
		const Location*		location = request.getLocation();
		const std::string&	locationName = location->getLocationName();
		std::string			aliasUri = request.getUri();

		if (location->getAlias().empty())
			return (false);
		aliasUri.replace(0, locationName.size(), location->getAlias());
		if (aliasUri[0] == '/')
			_requestedFilename = aliasUri;
		else {
			if (*_requestedFilename.rbegin() != '/')
				_requestedFilename += '/';
			_requestedFilename += aliasUri;
		}
		return (true);
	}

	int	Response::_statCgiScript(const Request& request)
	{
		struct stat		fileInfos;

		LOG_DEBUG("CGI filename: \"" << _requestedFilename << "\"");
		memset(&fileInfos, 0, sizeof(fileInfos));
		if (stat(_requestedFilename.c_str(), &fileInfos) < 0) {
			_logError(request, "stat()", "failed");
			return (500);
		} else if (S_ISDIR(fileInfos.st_mode)) {
			LOG_DEBUG("CGI dir");
			_loadDirLocation(request);
			return (301);
		} else if (!S_ISREG(fileInfos.st_mode)) {
			_logError(request, "", "is not a regular file");
			return (404);
		}
		return (_checkConditionalHeaders(request, &fileInfos));
	}

	void	Response::_prepareCgiOutputParsing(FILE* cgiOutputFile)
	{
		std::rewind(cgiOutputFile);
		_contentType.clear();
		_location.clear();
	}

	bool	Response::_isFieldName(const char c) const
	{
		if (!std::isprint(c) || c == '(' || c == ')' || c == '<' || c == '>'
				|| c == '@' || c == ',' || c == ';' || c == ':' || c == '\\'
				|| c == '"' || c == '/' || c == '[' || c == ']' || c == '?'
				|| c == '=' || c == '{' || c == '}' || c == ' ' || c == '\t')
			return (false) ;
		return (true);
	}

	bool	Response::_isFieldValue(const char c) const
	{
		if (std::isprint(c) || c == '\t')
			return (true) ;
		return (false);
	}

	bool	Response::_loadCgiHeaderFields(const Request& request,
											const char* buffer,
											std::string& fieldName,
											std::string& fieldValue) const
	{
		int				i = 0;
		int				startValue = 0;

		while (_isFieldName(buffer[i]))
			++i;
		if (buffer[i] != ':' || i == 0) {
			_logError(request, "Wrong character in CGI header:", "", buffer);
			return (false);
		}
		fieldName.assign(buffer, i++);
		while (_isFieldValue(buffer[i])) {
			if (!startValue && buffer[i] != ' ' && buffer[i] != '\t')
				startValue = i;
			++i;
		}
		if ((buffer[i] && buffer[i] != '\n' && buffer[i] != '\r')
				|| (buffer[i] == '\r' && buffer[i + 1] != '\n')) {
			_logError(request, "Wrong character in CGI header:", "", buffer);
			return (false);
		} else if (startValue) {
			while (buffer[i - 1] == ' ' || buffer[i - 1] == '\t')
				--i;
			fieldValue.assign(&buffer[startValue], i - startValue);
		}
		return (true);
	}

	bool	Response::_parseCgiContentType(const Request& request,
											const std::string& fieldValue)
	{
		if (!_contentType.empty()) {
			_logError(request, "This CGI header was returned twice:", "",
					"Content-Type");
			return (false);
		}
		_contentType = fieldValue;
		return (true);
	}

	bool	Response::_parseCgiLocalLocation(const Request& request,
												const std::string& fieldValue)
	{
		if (checkUriPathAbs(fieldValue.c_str()) == std::string::npos) {
			_logError(request, "Incorrect CGI's Location header value:", "",
					fieldValue.c_str());
			return (false);
		}
		_location = fieldValue;
		return (true);
	}

	bool	Response::_checkCgiClientLocation(const std::string& fieldValue)
	{
		const char*	str = fieldValue.c_str();
		size_t		i = checkUriScheme(str, ":");

		if (i == std::string::npos)
			return (false);
		str += i + 1;
		i = checkUriAuthority(str, "/?");
		if (i == std::string::npos)
			return (false);
		str += i;
		i = checkUriPath(str, (i != 0), false, "?#");
		if (i == std::string::npos)
			return (false);
		str += i;
		if (*str == '?') {
			i = checkUriQuery(++str, "#");
			if (i == std::string::npos)
				return (false);
			str += i;
		}
		if (*str == '#')
			i = checkUriFragment(++str, "");
		if (i == std::string::npos)
			return (false);
		return (true);
	}

	bool	Response::_parseCgiLocation(const Request& request,
										const std::string& fieldValue)
	{
		if (!_location.empty()) {
			_logError(request, "This CGI header was returned twice:", "",
					"Location");
			return (false);
		}
		if (fieldValue[0] == '/')
			return (_parseCgiLocalLocation(request, fieldValue));
		if (!_checkCgiClientLocation(fieldValue)) {
			_logError(request, "Incorrect CGI's Location header value:", "",
					fieldValue.c_str());
			return (false);
		}
		_location = fieldValue;
		return (true);
	}

	bool	Response::_parseCgiStatus(const Request& request, CgiHandler& cgi,
										const std::string& fieldValue) const
	{
		int		i = 0;

		if (cgi.responseCode != 0) {
			_logError(request, "This CGI header was returned twice:", "",
					"Status");
			return (false);
		}
		while (std::isdigit(fieldValue[i])) {
			if (++i == 4)
				break ;
		}
		if (i != 3) {
			_logError(request, "Incorrect CGI's Status header value:", "",
					fieldValue.c_str());
			return (false);
		}
		cgi.responseCode = std::atoi(std::string(fieldValue, 0, 3).c_str());
		if (!fieldValue[i])
			return (true);
		if (fieldValue[i] != ' ' || !std::isprint(fieldValue[i + 1])) {
			_logError(request, "Incorrect CGI's Status header value:", "",
					fieldValue.c_str());
			return (false);
		}
		return (true);
	}

	bool	Response::_parseCgiContentLength(const Request& request,
												const std::string& fieldValue)
	{
		int		i = 0;

		if (_contentLength != -1) {
			_logError(request, "This CGI header was returned twice:", "",
					"Content-Length");
			return (false);
		}
		while (std::isdigit(fieldValue[i]))
			++i;
		if (fieldValue[i]) {
			_logError(request, "Incorrect CGI's Content-Length header value:",
					"", fieldValue.c_str());
			return (false);
		}
		errno = 0;
		_contentLength = std::strtoll(fieldValue.c_str(), 0, 10);
		if (_contentLength == std::numeric_limits<int64_t>::max()
				|| errno != 0) {
			_logError(request, "Incorrect", "CGI's Content-Length header value",
					fieldValue.c_str());
			return (false);
		}
		return (true);
	}

	bool	Response::_recordCgiHeader(const Request& request, CgiHandler& cgi,
										const std::string& fieldName,
										const std::string& fieldValue,
										const char* buffer)
	{
		if (ft_strcmp_icase(fieldName, "Content-Type")) {
			if (!_parseCgiContentType(request, fieldValue))
				return (false);
		} else if (ft_strcmp_icase(fieldName, "Location")) {
			if (!_parseCgiLocation(request, fieldValue))
				return (false);
		} else if (ft_strcmp_icase(fieldName, "Status")) {
			if (!_parseCgiStatus(request, cgi, fieldValue))
				return (false);
		} else if (ft_strcmp_icase(fieldName, "Content-Length")) {
			if (!_parseCgiContentLength(request, fieldValue))
				return (false);
		} else {
			cgi.headers += buffer;
			if (cgi.headers[cgi.headers.size() - 2] != '\r')
				cgi.headers.insert(cgi.headers.end() - 1, '\r');
		}
		return (true);
	}

	bool	Response::_parseCgiHeader(const Request& request, CgiHandler& cgi,
										const char* buffer)
	{
		std::string		fieldName;
		std::string		fieldValue;

		if (buffer == std::string(CRLF) || buffer == std::string("\n")) {
			LOG_DEBUG("All CGI headers received");
			cgi.hasReadHeaders = true;
			return (true);
		}
		if (!_loadCgiHeaderFields(request, buffer, fieldName, fieldValue))
			return (false);
		if (!fieldValue.empty()) {
			LOG_DEBUG("CGI header: " << buffer);
			if (!_recordCgiHeader(request, cgi, fieldName, fieldValue, buffer))
				return (false);
			++cgi.nbHeaders;
		} else
			LOG_DEBUG("CGI header ignored: " << buffer);
		return (true);
	}

	int	Response::_readCgiHeaders(const Request& request, CgiHandler& cgi,
									char* buffer)
	{
		while (!std::feof(cgi.getOutputFile()) && !cgi.hasReadHeaders) {
			if (std::fgets(buffer, 4096, cgi.getOutputFile()) == 0)
				break ;
			if (!_parseCgiHeader(request, cgi, buffer))
				return (502);
		}
		if (std::ferror(cgi.getOutputFile())) {
			_logError(request, "Error with", "while reading CGI", "fgets()");
			return (500);
		}
		return (0);
	}

	bool	Response::_checkCgiEof(FILE* cgiOutputFile) const
	{
		int		c;

		if (std::feof(cgiOutputFile))
			return (true);
		c = std::fgetc(cgiOutputFile);
		if (std::feof(cgiOutputFile))
			return (true);
		if (std::ferror(cgiOutputFile))
			return (false);
		std::ungetc(c, cgiOutputFile);
		return (false);
	}

	bool	Response::_checkCgiLocation(const Request& request,
										CgiHandler& cgi) const
	{
		bool	hasError = false;

		if (!_location.empty() && _location[0] == '/'
				&& (cgi.nbHeaders != 1 || cgi.hasBody)) {
			_logError(request, "Wrong CGI Local Redirect while serving", "");
			return (false);
		}
		if (!_location.empty() && _location[0] != '/') {
			if (!cgi.hasBody && cgi.nbHeaders != 1)
				hasError = true;
			else if (cgi.hasBody && (_contentType.empty()
						|| (cgi.responseCode != 301 && cgi.responseCode != 302
							&& cgi.responseCode != 303
							&& cgi.responseCode != 307
							&& cgi.responseCode != 308)))
				hasError = true;
		}
		if (hasError) {
			_logError(request, "Wrong CGI Client Redirect while serving", "");
			return (false);
		}
		return (true);
	}

	bool	Response::_checkCgiHeaders(const Request& request, CgiHandler& cgi)
	{
		cgi.hasBody = !_checkCgiEof(cgi.getOutputFile());
		if (std::ferror(cgi.getOutputFile())) {
			_logError(request, "Error with", "while reading CGI", "fgetc()");
			return (false);
		} else if (!cgi.hasReadHeaders || cgi.nbHeaders == 0) {
			_logError(request, "Incomplete CGI headers while serving", "");
			return (false);
		} else if (cgi.hasBody && _contentType.empty()) {
			_logError(request, "CGI Document Response misses header", "",
					"Content-Type");
			return (false);
		} else if (cgi.hasBody && _contentLength != -1) {
			LOG_WARN("The CGI sent a \"Content-Length\" header without data");
			_contentLength = -1;
		} else if (!_checkCgiLocation(request, cgi))
			return (false);
		if (!cgi.hasBody)
			LOG_DEBUG("CGI output reached EOF");
		return (true);
	}

	bool	Response::_createTmpCgiBodyFile(const Request& request)
	{
		_tmpCgiBodyFilename = createRandomFilename();
		LOG_DEBUG("CGI tmp body filename: \"" << _tmpCgiBodyFilename << "\"");
		_requestedFile.open(_tmpCgiBodyFilename.c_str(), std::ios::in
				| std::ios::out | std::ios::binary);
		if (_requestedFile.fail()) {
			_logError(request, "open()", "failed", _tmpCgiBodyFilename.c_str());
			return (false);
		}
		LOG_DEBUG("CGI tmp body file created");
		return (true);
	}

	bool	Response::_checkCgiBodyLength(const Request& request,
											const char* filename)
	{
		struct stat		fileInfos;
		int64_t			fileSize;

		memset(&fileInfos, 0, sizeof(fileInfos));
		if (stat(filename, &fileInfos) < 0) {
			_logError(request, "stat()", "failed", filename);
			return (false);
		} else if (!S_ISREG(fileInfos.st_mode)) {
			_logError(request, "", "is not a regular file", filename);
			return (false);
		}
		fileSize = fileInfos.st_size;
		if (_contentLength != fileSize) {
			LOG_WARN("The CGI sent more data (" << fileSize
					<< ") than specified in its \"Content-Length\" header ("
					<< _contentLength << ")");
			_contentLength = fileSize;
		}
		return (true);
	}

	bool	Response::_loadTmpCgiBodyFile(const Request& request,
											FILE* cgiOutputFile)
	{
		char		buffer[4096];
		size_t		bytesRead;

		if (!_createTmpCgiBodyFile(request))
			return (false);
		while (!std::feof(cgiOutputFile)) {
			bytesRead = std::fread(buffer, sizeof(char), 4095, cgiOutputFile);
			if (std::ferror(cgiOutputFile)) {
				_logError(request, "Error in", "while reading CGI", "fread()");
				return (false);
			}
			buffer[bytesRead] = '\0';
			_requestedFile << buffer;
		}
		LOG_DEBUG("CGI body recorded in: " << _tmpCgiBodyFilename);
		_requestedFile.seekg(0, std::ios::beg);
		if (_requestedFile.fail()) {
			_logError(request, "seekg()", "failed",
					_tmpCgiBodyFilename.c_str());
			return (false);
		}
		return (_checkCgiBodyLength(request, _tmpCgiBodyFilename.c_str()));
	}

	void	Response::_loadCgiHeaders(const Request& request, CgiHandler& cgi)
	{
		if (!_location.empty() && _location[0] != '/' && !cgi.hasBody)
			cgi.responseCode = 302;
		else if (cgi.responseCode == 0 && _responseCode == 0)
			cgi.responseCode = 200;
		if (cgi.responseCode != 200 || _responseCode == 0)
			_responseCode = cgi.responseCode;
		if (_contentType.empty())
			_contentType = _getContentType("");
		_loadHeaders(request);
		if (!cgi.headers.empty()) {
			if (_responseBuffer.size() > 4 && (_responseBuffer.c_str()
				   + (_responseBuffer.size() - 4) == std::string(CRLF CRLF)))
				_responseBuffer.erase(_responseBuffer.size() - 2);
			_responseBuffer += cgi.headers + CRLF;
			LOG_DEBUG("Added CGI Headers:\n" << cgi.headers << CRLF);
		}
	}

	int	Response::_processCgiOutput(Request& request, CgiHandler& cgi)
	{
		char			buffer[4096];
		int				errorCode;

		_prepareCgiOutputParsing(cgi.getOutputFile());
		errorCode = _readCgiHeaders(request, cgi, buffer);
		if (errorCode)
			return (errorCode);
		if (!_checkCgiHeaders(request, cgi))
			return (502);
		if (!_location.empty() && _location[0] == '/')
			return (_loadInternalRedirect(request, _location));
		if (cgi.hasBody) {
			if (!_loadTmpCgiBodyFile(request, cgi.getOutputFile()))
				return (500);
			_isFileResponse = true;
		}
		_loadCgiHeaders(request, cgi);
		return (0);
	}

	int	Response::_loadCgiPass(Request& request) try
	{
		// TO DO: Check leaks of sockets/fds/Logger in the child process?

		CgiHandler		cgi(request.getLocation()->getCgiPass(),
							_requestedFilename);
		int				responseCode;

		responseCode = _statCgiScript(request);
		if (responseCode)
			return (responseCode);
		cgi.loadEnv(request, _getContentType(request.getExtension()));
		if (!cgi.prepareCgiIo(request))
			return (500);
		responseCode = cgi.launchCgiProcess(request);
		if (responseCode)
			return (responseCode);
		responseCode = _processCgiOutput(request, cgi);
		if (responseCode)
			return (responseCode);
		return (0);
	}
	catch (...) {
		throw ;
	}

	int	Response::_loadRequestedFile(Request& request)
	{
		if (!_loadFileWithAlias(request)) {
			if (request.getLocation()->getRoot()[0] == '/')
				_requestedFilename = request.getLocation()->getRoot();
			else {
				if (*_requestedFilename.rbegin() != '/')
					_requestedFilename += '/';
				_requestedFilename += request.getLocation()->getRoot();
			}
			if (*_requestedFilename.rbegin() == '/')
				_requestedFilename.erase(_requestedFilename.end() - 1);
			_requestedFilename += request.getUri();
		}
		if (request.getRequestMethod() == "DELETE")
			return (_removeRequestedFile(request));
		if (!request.getLocation()->getCgiPass().empty())
			return (_loadCgiPass(request));
		if (request.getRequestMethod() == "POST")
			return (_postRequestBody(request));
		if (*_requestedFilename.rbegin() == '/')
			return (_loadIndex(request));
		return (_openRequestedFile(request));
	}

	std::string	Response::_getPostResponseBody(const Request& request)
	{
		std::ostringstream		postResponseBody;

		if (_responseCode == 201 && request.getLocation()->getCgiPass().empty())
			postResponseBody << "<html>" CRLF
				"<head><title>Your file has been saved!</title></head>" CRLF
				"<body>" CRLF
				"<center><h1>Click <A href=\""
				<< _escapeUriComponent(request.getUri())
				<< "\">here</A> to view it.</h1></center>" CRLF
				"<hr><center>webserv</center>" CRLF
				"</body>" CRLF
				"</html>" CRLF;
		return (postResponseBody.str());
	}

	bool	Response::_handleBodyDrop(const Request& request)
	{
		if (request.getRequestMethod() == "HEAD"
				|| _responseCode == 204 || _responseCode == 304
				|| (_responseCode >= 100 && _responseCode < 200)) {
			LOG_DEBUG("HTTP set discard body");
			_isChunkedResponse = false;
			_isFileResponse = false;
			return (true);
		}
		return (false);
	}

	void	Response::prepareResponse(Request& request)
	{
		// TO DO: Evaluate If-xxx conditional headers

		int		responseCode;

		if (_isResponseReady)
			return ;
		if (request.getLocation()->getReturnPair().first >= 0)
			return (prepareErrorResponse(request,
						request.getLocation()->getReturnPair().first));
		if (_isChunkedResponse)
			return (_prepareChunkedResponse(request));
		if (_isFileResponse)
			return (_prepareFileResponse(request));
		responseCode = _loadRequestedFile(request);
		if (responseCode == INTERNAL_REDIRECT)
			return ;
		if (responseCode != 0)
			return (prepareErrorResponse(request, responseCode));
		_loadHeaders(request);
		if (!_handleBodyDrop(request) && request.getRequestMethod() == "POST"
				&& request.getLocation()->getCgiPass().empty())
			_responseBuffer += _getPostResponseBody(request);
		_isResponseReady = true;
	}

	bool	Response::_loadErrorPage(Request& request)
	{
		Location::error_page_map::const_iterator	errorPage;
		int											errorCode;

		if (request.isInternalRedirect())
			return (false);
		errorPage = request.getLocation()->getErrorPages().find(_responseCode);
		if (errorPage != request.getLocation()->getErrorPages().end()) {
			if (errorPage->second[0] == '/') {
				if (request.getRequestMethod() != "GET"
						&& request.getRequestMethod() != "HEAD")
					request.setRequestMethod("GET");
				errorCode = request.loadInternalRedirect(errorPage->second);
				if (errorCode != 0) {
					prepareErrorResponse(request, errorCode);
				} else
					prepareResponse(request);
				return (true);
			} else {
				_responseCode = 302;
				_location = errorPage->second;
			}
		}
		return (false);
	}

	void	Response::_loadRelativeLocationPrefix(const Request& request)
	{
		std::string	serverPort = to_string(ntohs(request.getServerConfig()
												->getListenPair().sin_port));

		_location = std::string("http://") + request.getHost();
		if (serverPort != "80")
			_location += std::string(":") + serverPort;
	}

	bool	Response::_loadReturn(const Request& request)
	{
		int			returnCode = request.getLocation()->getReturnPair().first;
		std::string	returnText = request.getLocation()->getReturnPair().second;
		std::string	serverPort = to_string(ntohs(request.getServerConfig()
												->getListenPair().sin_port));

		if (returnCode == 444 && returnText.empty()) {
			_isKeepAlive = false;
			_isResponseReady = true;
			return (true);
		}
		if (returnCode == 301 || returnCode == 302 || returnCode == 303
				|| returnCode == 307 || returnCode == 308) {
			if (!returnText.empty() && returnText[0] == '/')
				_loadRelativeLocationPrefix(request);
			_location += returnText;
			return (false);
		} else if (returnCode < 0 || returnText.empty())
			return (false);
		_loadHeaders(request);
		if (!_handleBodyDrop(request))
			_responseBuffer += returnText;
		_isResponseReady = true;
		return (true);
	}

	void	Response::prepareErrorResponse(Request& request, int errorCode)
	{
		const std::string*	specialBody = &Response::_getSpecialResponseBody(0);

		if (errorCode == 0 && (_responseCode < 300 || _responseCode > 599))
			errorCode = 500;
		else if (errorCode == 0)
			errorCode = _responseCode;
		clearResponse(request, errorCode);
		if (request.getLocation() && _loadReturn(request))
			return ;
		_isKeepAlive = isKeepAlive();
		if (request.getLocation() && _loadErrorPage(request))
			return ;
		specialBody = &(Response::_getSpecialResponseBody(_responseCode));
		if (!specialBody->empty()) {
			_contentType = "text/html";
			_contentLength = static_cast<int64_t>(specialBody->length());
		}
		_loadHeaders(request);
		if (!_handleBodyDrop(request))
			_responseBuffer += *specialBody;
		_isResponseReady = true;
	}

	void	Response::clearResponse(const Request& request,
									int responseCodeToKeep)
	{
		_responseBuffer.clear();
		_requestedFilename = XSTR(WEBSERV_ROOT);
		_tmpCgiBodyFilename.clear();
//		if (_requestedFileFd >= 0)
		if (_requestedFile.is_open())
			_closeRequestedFile();
		if (_fileBuffer != 0) {
			delete[] _fileBuffer;
			_fileBuffer = 0;
		}
		_fileBufferSize = 0;
		_bufferPos = 0;
		if (_indexDirectory != 0)
			_closeIndexDirectory();
		_dirEntrySet.clear();
		_responseCode = responseCodeToKeep;
		_contentType = "application/octet-stream";
		_contentLength = -1;
		if (responseCodeToKeep != 304)
			_lastModifiedTime = -1;
		_isKeepAlive = request.isKeepAlive();
		if (responseCodeToKeep == 0)
			_location.clear();
		_isChunkedResponse = false;
		_isFileResponse = false;
		_isResponseReady = false;
	}

	/**************************************************************************/
	/*                         STATIC MEMBER FUNCTIONS                        */
	/**************************************************************************/

	const std::string&	Response::_getDate(time_t lastModifiedTime)
	{
		static char			buffer[32];
		static std::string	date(32, '\0');
		time_t				rawTime;
		struct tm*			gmtTime;

		if (lastModifiedTime != -1)
			gmtTime = std::gmtime(&lastModifiedTime);
		else {
			rawTime = std::time(0);
			gmtTime = std::gmtime(&rawTime);
		}
		std::strftime(buffer, 32, "%a, %d %b %Y %H:%M:%S GMT", gmtTime);
		date = buffer;
		return (date);
	}

	void	Response::initResponseMaps()
	{
		(void)Response::_getResponseStatus(0);
		(void)Response::_getSpecialResponseBody(0);
		(void)Response::_getContentType("");
	}

	const std::string&	Response::_getResponseStatus(int responseCode)
	{
		static std::string									emptyStatus("");
		static std::map<int, std::string>					statusMap;
		static std::map<int, std::string>::const_iterator	responseStatus;

		if (statusMap.empty()) {
			statusMap[200] = "OK";
			statusMap[201] = "Created";
			statusMap[202] = "Accepted";
			statusMap[204] = "No Content";
			statusMap[206] = "Partial Content";
			statusMap[301] = "Moved Permanently";
			statusMap[302] = "Moved Temporarily";
			statusMap[303] = "See Other";
			statusMap[304] = "Not Modified";
			statusMap[307] = "Temporary Redirect";
			statusMap[308] = "Permanent Redirect";
			statusMap[400] = "Bad Request";
			statusMap[401] = "Unauthorized";
			statusMap[402] = "Payment Required";
			statusMap[403] = "Forbidden";
			statusMap[404] = "Not Found";
			statusMap[405] = "Not Allowed";
			statusMap[406] = "Not Acceptable";
			statusMap[408] = "Request Time-out";
			statusMap[409] = "Conflict";
			statusMap[410] = "Gone";
			statusMap[411] = "Length Required";
			statusMap[412] = "Precondition Failed";
			statusMap[413] = "Request Entity Too Large";
			statusMap[414] = "Request-URI Too Large";
			statusMap[415] = "Unsupported Media Type";
			statusMap[416] = "Requested Range Not Satisfiable";
			statusMap[421] = "Misdirected Request";
			statusMap[429] = "Too Many Requests";
			statusMap[500] = "Internal Server Error";
			statusMap[501] = "Not Implemented";
			statusMap[502] = "Bad Gateway";
			statusMap[503] = "Service Temporarily Unavailable";
			statusMap[504] = "Gateway Time-out";
			statusMap[505] = "HTTP Version Not Supported";
			statusMap[507] = "Insufficient Storage";
		}
		if (responseCode < 200 || responseCode > 507)
			return (emptyStatus);
		responseStatus = statusMap.find(responseCode);
		if (responseStatus != statusMap.end())
			return (responseStatus->second);
		return (emptyStatus);
	}

	const std::string&	Response::_getSpecialResponseBody(int responseCode)
	{
		static std::string									emptyBody("");
		static std::map<int, std::string>					bodyMap;
		static std::map<int, std::string>::const_iterator	responseBody;

		if (bodyMap.empty()) {
			bodyMap[301] = "<html>" CRLF
			"<head><title>301 Moved Permanently</title></head>" CRLF
			"<body>" CRLF "<center><h1>301 Moved Permanently</h1></center>" CRLF
			"<hr><center>webserv</center>" CRLF "</body>" CRLF "</html>"
			CRLF;
			bodyMap[302] = "<html>" CRLF
			"<head><title>302 Found</title></head>" CRLF
			"<body>" CRLF "<center><h1>302 Found</h1></center>" CRLF
			"<hr><center>webserv</center>" CRLF "</body>" CRLF "</html>"
			CRLF;
			bodyMap[303] = "<html>" CRLF
			"<head><title>303 See Other</title></head>" CRLF
			"<body>" CRLF "<center><h1>303 See Other</h1></center>" CRLF
			"<hr><center>webserv</center>" CRLF "</body>" CRLF "</html>"
			CRLF;
			bodyMap[307] = "<html>" CRLF
			"<head><title>307 Temporary Redirect</title></head>" CRLF
			"<body>" CRLF "<center><h1>307 Temporary Redirect</h1></center>" CRLF
			"<hr><center>webserv</center>" CRLF "</body>" CRLF "</html>"
			CRLF;
			bodyMap[308] = "<html>" CRLF
			"<head><title>308 Permanent Redirect</title></head>" CRLF
			"<body>" CRLF "<center><h1>308 Permanent Redirect</h1></center>" CRLF
			"<hr><center>webserv</center>" CRLF "</body>" CRLF "</html>"
			CRLF;
			bodyMap[400] = "<html>" CRLF
			"<head><title>400 Bad Request</title></head>" CRLF
			"<body>" CRLF "<center><h1>400 Bad Request</h1></center>" CRLF
			"<hr><center>webserv</center>" CRLF "</body>" CRLF "</html>"
			CRLF;
			bodyMap[401] = "<html>" CRLF
			"<head><title>401 Authorization Required</title></head>" CRLF
			"<body>" CRLF "<center><h1>401 Authorization Required</h1></center>" CRLF
			"<hr><center>webserv</center>" CRLF "</body>" CRLF "</html>"
			CRLF;
			bodyMap[402] = "<html>" CRLF
			"<head><title>402 Payment Required</title></head>" CRLF
			"<body>" CRLF "<center><h1>402 Payment Required</h1></center>" CRLF
			"<hr><center>webserv</center>" CRLF "</body>" CRLF "</html>"
			CRLF;
			bodyMap[403] = "<html>" CRLF
			"<head><title>403 Forbidden</title></head>" CRLF
			"<body>" CRLF "<center><h1>403 Forbidden</h1></center>" CRLF
			"<hr><center>webserv</center>" CRLF "</body>" CRLF "</html>"
			CRLF;
			bodyMap[404] = "<html>" CRLF
			"<head><title>404 Not Found</title></head>" CRLF
			"<body>" CRLF "<center><h1>404 Not Found</h1></center>" CRLF
			"<hr><center>webserv</center>" CRLF "</body>" CRLF "</html>"
			CRLF;
			bodyMap[405] = "<html>" CRLF
			"<head><title>405 Not Allowed</title></head>" CRLF
			"<body>" CRLF "<center><h1>405 Not Allowed</h1></center>" CRLF
			"<hr><center>webserv</center>" CRLF "</body>" CRLF "</html>"
			CRLF;
			bodyMap[406] = "<html>" CRLF
			"<head><title>406 Not Acceptable</title></head>" CRLF
			"<body>" CRLF "<center><h1>406 Not Acceptable</h1></center>" CRLF
			"<hr><center>webserv</center>" CRLF "</body>" CRLF "</html>"
			CRLF;
			bodyMap[408] = "<html>" CRLF
			"<head><title>408 Request Time-out</title></head>" CRLF
			"<body>" CRLF "<center><h1>408 Request Time-out</h1></center>" CRLF
			"<hr><center>webserv</center>" CRLF "</body>" CRLF "</html>"
			CRLF;
			bodyMap[409] = "<html>" CRLF
			"<head><title>409 Conflict</title></head>" CRLF
			"<body>" CRLF "<center><h1>409 Conflict</h1></center>" CRLF
			"<hr><center>webserv</center>" CRLF "</body>" CRLF "</html>"
			CRLF;
			bodyMap[410] = "<html>" CRLF
			"<head><title>410 Gone</title></head>" CRLF
			"<body>" CRLF "<center><h1>410 Gone</h1></center>" CRLF
			"<hr><center>webserv</center>" CRLF "</body>" CRLF "</html>"
			CRLF;
			bodyMap[411] = "<html>" CRLF
			"<head><title>411 Length Required</title></head>" CRLF
			"<body>" CRLF "<center><h1>411 Length Required</h1></center>" CRLF
			"<hr><center>webserv</center>" CRLF "</body>" CRLF "</html>"
			CRLF;
			bodyMap[412] = "<html>" CRLF
			"<head><title>412 Precondition Failed</title></head>" CRLF
			"<body>" CRLF "<center><h1>412 Precondition Failed</h1></center>" CRLF
			"<hr><center>webserv</center>" CRLF "</body>" CRLF "</html>"
			CRLF;
			bodyMap[413] = "<html>" CRLF
			"<head><title>413 Request Entity Too Large</title></head>" CRLF
			"<body>" CRLF "<center><h1>413 Request Entity Too Large</h1></center>" CRLF
			"<hr><center>webserv</center>" CRLF "</body>" CRLF "</html>"
			CRLF;
			bodyMap[414] = "<html>" CRLF
			"<head><title>414 Request-URI Too Large</title></head>" CRLF
			"<body>" CRLF "<center><h1>414 Request-URI Too Large</h1></center>" CRLF
			"<hr><center>webserv</center>" CRLF "</body>" CRLF "</html>"
			CRLF;
			bodyMap[415] = "<html>" CRLF
			"<head><title>415 Unsupported Media Type</title></head>" CRLF
			"<body>" CRLF "<center><h1>415 Unsupported Media Type</h1></center>" CRLF
			"<hr><center>webserv</center>" CRLF "</body>" CRLF "</html>"
			CRLF;
			bodyMap[416] = "<html>" CRLF
			"<head><title>416 Requested Range Not Satisfiable</title></head>" CRLF
			"<body>" CRLF "<center><h1>416 Requested Range Not Satisfiable</h1></center>" CRLF
			"<hr><center>webserv</center>" CRLF "</body>" CRLF "</html>"
			CRLF;
			bodyMap[421] = "<html>" CRLF
			"<head><title>421 Misdirected Request</title></head>" CRLF
			"<body>" CRLF "<center><h1>421 Misdirected Request</h1></center>" CRLF
			"<hr><center>webserv</center>" CRLF "</body>" CRLF "</html>"
			CRLF;
			bodyMap[429] = "<html>" CRLF
			"<head><title>429 Too Many Requests</title></head>" CRLF
			"<body>" CRLF "<center><h1>429 Too Many Requests</h1></center>" CRLF
			"<hr><center>webserv</center>" CRLF "</body>" CRLF "</html>"
			CRLF;
			bodyMap[494] = "<html>" CRLF
			"<head><title>400 Request Header Or Cookie Too Large</title></head>" CRLF
			"<body>" CRLF "<center><h1>400 Bad Request</h1></center>" CRLF
			"<center>Request Header Or Cookie Too Large</center>" CRLF
			"<hr><center>webserv</center>" CRLF "</body>" CRLF "</html>"
			CRLF;
			bodyMap[495] = "<html>" CRLF
			"<head><title>400 The SSL certificate error</title></head>" CRLF
			"<body>" CRLF "<center><h1>400 Bad Request</h1></center>" CRLF
			"<center>The SSL certificate error</center>" CRLF
			"<hr><center>webserv</center>" CRLF "</body>" CRLF "</html>"
			CRLF;
			bodyMap[496] = "<html>" CRLF
			"<head><title>400 No required SSL certificate was sent</title></head>"
			CRLF "<body>" CRLF "<center><h1>400 Bad Request</h1></center>" CRLF
			"<center>No required SSL certificate was sent</center>" CRLF
			"<hr><center>webserv</center>" CRLF "</body>" CRLF "</html>"
			CRLF;
			bodyMap[497] = "<html>" CRLF
			"<head><title>400 The plain HTTP request was sent to HTTPS port</title></head>"
			CRLF "<body>" CRLF "<center><h1>400 Bad Request</h1></center>" CRLF
			"<center>The plain HTTP request was sent to HTTPS port</center>" CRLF
			"<hr><center>webserv</center>" CRLF "</body>" CRLF "</html>"
			CRLF;
			bodyMap[498] = "<html>" CRLF
			"<head><title>404 Not Found</title></head>" CRLF
			"<body>" CRLF "<center><h1>404 Not Found</h1></center>" CRLF
			"<hr><center>webserv</center>" CRLF "</body>" CRLF "</html>"
			CRLF;
			bodyMap[500] = "<html>" CRLF
			"<head><title>500 Internal Server Error</title></head>" CRLF
			"<body>" CRLF "<center><h1>500 Internal Server Error</h1></center>" CRLF
			"<hr><center>webserv</center>" CRLF "</body>" CRLF "</html>"
			CRLF;
			bodyMap[501] = "<html>" CRLF
			"<head><title>501 Not Implemented</title></head>" CRLF
			"<body>" CRLF "<center><h1>501 Not Implemented</h1></center>" CRLF
			"<hr><center>webserv</center>" CRLF "</body>" CRLF "</html>"
			CRLF;
			bodyMap[502] = "<html>" CRLF
			"<head><title>502 Bad Gateway</title></head>" CRLF
			"<body>" CRLF "<center><h1>502 Bad Gateway</h1></center>" CRLF
			"<hr><center>webserv</center>" CRLF "</body>" CRLF "</html>"
			CRLF;
			bodyMap[503] = "<html>" CRLF
			"<head><title>503 Service Temporarily Unavailable</title></head>" CRLF
			"<body>" CRLF "<center><h1>503 Service Temporarily Unavailable</h1></center>" CRLF
			"<hr><center>webserv</center>" CRLF "</body>" CRLF "</html>"
			CRLF;
			bodyMap[504] = "<html>" CRLF
			"<head><title>504 Gateway Time-out</title></head>" CRLF
			"<body>" CRLF "<center><h1>504 Gateway Time-out</h1></center>" CRLF
			"<hr><center>webserv</center>" CRLF "</body>" CRLF "</html>"
			CRLF;
			bodyMap[505] = "<html>" CRLF
			"<head><title>505 HTTP Version Not Supported</title></head>" CRLF
			"<body>" CRLF "<center><h1>505 HTTP Version Not Supported</h1></center>" CRLF
			"<hr><center>webserv</center>" CRLF "</body>" CRLF "</html>"
			CRLF;
			bodyMap[507] = "<html>" CRLF
			"<head><title>507 Insufficient Storage</title></head>" CRLF
			"<body>" CRLF "<center><h1>507 Insufficient Storage</h1></center>" CRLF
			"<hr><center>webserv</center>" CRLF "</body>" CRLF "</html>"
			CRLF;
		}
		if (responseCode < 301 || responseCode > 507)
			return (emptyBody);
		responseBody = bodyMap.find(responseCode);
		if (responseBody != bodyMap.end())
			return (responseBody->second);
		return (emptyBody);
	}

	const std::string&	Response::_getContentType(const std::string& extension)
	{
		typedef std::map<std::string, std::string>	_type_map;
		typedef _type_map::const_iterator			_type_map_it;

		static std::string		defaultType = "application/octet-stream";
		static _type_map		typeMap;
		static _type_map_it		contentType;
		std::string				ext;

		if (typeMap.empty()) {
			typeMap["html"] = "text/html";
			typeMap["htm"] = "text/html";
			typeMap["shtml"] = "text/html";
			typeMap["css"] = "text/css";
			typeMap["xml"] = "text/xml";
			typeMap["gif"] = "image/gif";
			typeMap["jpeg"] = "image/jpeg";
			typeMap["jpg"] = "image/jpeg";
			typeMap["js"] = "application/javascript";
			typeMap["atom"] = "application/atom+xml";
			typeMap["rss"] = "application/rss+xml";
			typeMap["mml"] = "text/mathml";
			typeMap["txt"] = "text/plain";
			typeMap["jad"] = "text/vnd.sun.j2me.app-descriptor";
			typeMap["wml"] = "text/vnd.wap.wml";
			typeMap["htc"] = "text/x-component";
			typeMap["avif"] = "image/avif";
			typeMap["png"] = "image/png";
			typeMap["svg"] = "image/svg+xml";
			typeMap["svgz"] = "image/svg+xml";
			typeMap["tif"] = "image/tiff";
			typeMap["tiff"] = "image/tiff";
			typeMap["wbmp"] = "image/vnd.wap.wbmp";
			typeMap["webp"] = "image/webp";
			typeMap["ico"] = "image/x-icon";
			typeMap["jng"] = "image/x-jng";
			typeMap["bmp"] = "image/x-ms-bmp";
			typeMap["woff"] = "font/woff";
			typeMap["woff2"] = "font/woff2";
			typeMap["jar"] = "application/java-archive";
			typeMap["war"] = "application/java-archive";
			typeMap["ear"] = "application/java-archive";
			typeMap["json"] = "application/json";
			typeMap["hqx"] = "application/mac-binhex40";
			typeMap["doc"] = "application/msword";
			typeMap["pdf"] = "application/pdf";
			typeMap["ps"] = "application/postscript";
			typeMap["eps"] = "application/postscript";
			typeMap["ai"] = "application/postscript";
			typeMap["rtf"] = "application/rtf";
			typeMap["m3u8"] = "application/vnd.apple.mpegurl";
			typeMap["kml"] = "application/vnd.google-earth.kml+xml";
			typeMap["kmz"] = "application/vnd.google-earth.kmz";
			typeMap["xls"] = "application/vnd.ms-excel";
			typeMap["eot"] = "application/vnd.ms-fontobject";
			typeMap["ppt"] = "application/vnd.ms-powerpoint";
			typeMap["odg"] = "application/vnd.oasis.opendocument.graphics";
			typeMap["odp"] = "application/vnd.oasis.opendocument.presentation";
			typeMap["ods"] = "application/vnd.oasis.opendocument.spreadsheet";
			typeMap["odt"] = "application/vnd.oasis.opendocument.text";
			typeMap["pptx"] =
    "application/vnd.openxmlformats-officedocument.presentationml.presentation";
			typeMap["xlsx"] =
    "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet";
			typeMap["docx"] =
    "application/vnd.openxmlformats-officedocument.wordprocessingml.document";
			typeMap["wmlc"] = "application/vnd.wap.wmlc";
			typeMap["wasm"] = "application/wasm";
			typeMap["7z"] = "application/x-7z-compressed";
			typeMap["cco"] = "application/x-cocoa";
			typeMap["jardiff"] = "application/x-java-archive-diff";
			typeMap["jnlp"] = "application/x-java-jnlp-file";
			typeMap["run"] = "application/x-makeself";
			typeMap["pl"] = "application/x-perl";
			typeMap["pm"] = "application/x-perl";
			typeMap["prc"] = "application/x-pilot";
			typeMap["pdb"] = "application/x-pilot";
			typeMap["rar"] = "application/x-rar-compressed";
			typeMap["rpm"] = "application/x-redhat-package-manager";
			typeMap["sea"] = "application/x-sea";
			typeMap["swf"] = "application/x-shockwave-flash";
			typeMap["sit"] = "application/x-stuffit";
			typeMap["tcl"] = "application/x-tcl";
			typeMap["tk"] = "application/x-tcl";
			typeMap["der"] = "application/x-x509-ca-cert";
			typeMap["pem"] = "application/x-x509-ca-cert";
			typeMap["crt"] = "application/x-x509-ca-cert";
			typeMap["xpi"] = "application/x-xpinstall";
			typeMap["xhtml"] = "application/xhtml+xml";
			typeMap["xspf"] = "application/xspf+xml";
			typeMap["zip"] = "application/zip";
			typeMap["bin"] = "application/octet-stream";
			typeMap["exe"] = "application/octet-stream";
			typeMap["dll"] = "application/octet-stream";
			typeMap["deb"] = "application/octet-stream";
			typeMap["dmg"] = "application/octet-stream";
			typeMap["iso"] = "application/octet-stream";
			typeMap["img"] = "application/octet-stream";
			typeMap["msi"] = "application/octet-stream";
			typeMap["msp"] = "application/octet-stream";
			typeMap["msm"] = "application/octet-stream";
			typeMap["mid"] = "audio/midi";
			typeMap["midi"] = "audio/midi";
			typeMap["kar"] = "audio/midi";
			typeMap["mp3"] = "audio/mpeg";
			typeMap["ogg"] = "audio/ogg";
			typeMap["m4a"] = "audio/x-m4a";
			typeMap["ra"] = "audio/x-realaudio";
			typeMap["3gpp"] = "video/3gpp";
			typeMap["3gp"] = "video/3gpp";
			typeMap["ts"] = "video/mp2t";
			typeMap["mp4"] = "video/mp4";
			typeMap["mpeg"] = "video/mpeg";
			typeMap["mpg"] = "video/mpeg";
			typeMap["mov"] = "video/quicktime";
			typeMap["webm"] = "video/webm";
			typeMap["flv"] = "video/x-flv";
			typeMap["m4v"] = "video/x-m4v";
			typeMap["mng"] = "video/x-mng";
			typeMap["asx"] = "video/x-ms-asf";
			typeMap["asf"] = "video/x-ms-asf";
			typeMap["wmv"] = "video/x-ms-wmv";
			typeMap["avi"] = "video/x-msvideo";
		}
		if (extension.empty())
			return (defaultType);
		ext = ft_str_tolower(extension);
		contentType = typeMap.find(ext);
		if (contentType != typeMap.end())
			return (contentType->second);
		return (defaultType);
	}

}	// namespace webserv
