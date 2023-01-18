#include "core/Response.hpp"

#include <fcntl.h>
#include <unistd.h>

#include <map>
#include <cerrno>
#include <cstring>
#include <sstream>
#include <utility>
#include <cctype>

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

	Response::Response(): _requestedFileName(XSTR(WEBSERV_ROOT)),
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
									_requestedFileName(XSTR(WEBSERV_ROOT)),
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

	void	Response::setResponseCode(int responseCode)
	{
		_responseCode = responseCode;
	}

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

	const std::string	Response::_getETag() const
	{
		std::ostringstream		eTag;

		eTag << std::hex << "\"";
		if (static_cast<int64_t>(_lastModifiedTime) < 0)
			eTag << "-" << static_cast<uint64_t>(-_lastModifiedTime);
		else
			eTag << static_cast<uint64_t>(_lastModifiedTime);
		eTag << "-";
		if (_contentLength < 0)
			eTag << "-" << static_cast<uint64_t>(-_contentLength);
		else
			eTag << static_cast<uint64_t>(_contentLength);
		eTag << "\"";
		return (eTag.str());
	}

	void	Response::_loadHeaders()
	{
		// TO DO: Add "Allow" header when response is 405 with supported methods

		const char*			connection = _isKeepAlive ? "keep-alive" : "close";
		std::ostringstream	headers;

		headers << "HTTP/1.1 " << _responseCode << " "
			<< Response::_getResponseStatus(_responseCode) << CRLF
			<< "Server: webserv" << CRLF
			<< "Date: " << Response::_getDate() << CRLF
			<< "Content-Type: " << _contentType << CRLF;	// only if content?
		if (_contentLength != -1)
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
		if (filename[0] == '\0')
			filename = _requestedFileName.c_str();
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

		LOG_DEBUG("http autoindex file: " << entryName);
		if (entryName[0] == '.')
			return (0);
		memset(&fileInfos, 0, sizeof(fileInfos));
		if (stat(_requestedFileName.c_str(), &fileInfos) < 0) {
			if (errno != ENOENT && errno != ELOOP) {
				_logError(request, "stat()", "failed");
				if (errno == EACCES)
					return (0);
				return (500);
			}
			if (lstat(_requestedFileName.c_str(), &fileInfos) < 0) {
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
			LOG_ERROR("Bad closedir() on \"" << _requestedFileName << "\"");
		_indexDirectory = 0;
	}

	int	Response::_loadDirEntries(const Request& request)
	{
		struct dirent*	dirEntry;
		size_t			pathLen = _requestedFileName.size();
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
			_requestedFileName += '/';
			_requestedFileName += dirEntry->d_name;
			errorCode = _loadDirEntry(request, dirEntry->d_name);
			_requestedFileName[pathLen] = '\0';
			if (errorCode != 0)
				return (errorCode);
		}
		_closeIndexDirectory();
		return (0);
	}

	const std::string	Response::_escapeHtml(const std::string& str,
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

	const std::string
		Response::_escapeUriComponent(const std::string& uri) const
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

	const std::string
		Response::_escapeUri(const std::string& uri) const
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

	const std::string
		Response::_getFileDate(const struct stat& fileInfos) const
	{
		char			buffer[32];
		struct tm*		gmtTime = std::gmtime(&fileInfos.st_mtime);

		std::strftime(buffer, 32, "%d-%b-%Y %H:%M ", gmtTime);
		return (buffer);
	}

	const std::string
		Response::_getFileSize(const struct stat& fileInfos) const
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

	int	Response::_loadAutoindexHtml(const Request& request)
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
		return (0);
	}

	void	Response::_prepareChunkedResponse(Request& request)
	{
		int		errorCode;

		_clearBuffer();
		if (_indexDirectory != 0) {
			errorCode = _loadDirEntries(request);
			if (errorCode != 0)
				return (prepareErrorResponse(request, errorCode));
			errorCode = _loadAutoindexHtml(request);
			if (errorCode != 0)
				return (prepareErrorResponse(request, errorCode));
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
		// TO DO: Return error only after 10 identical redirect (like NGINX)?

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
		if (request.getRequestMethod() != "GET"
				|| !request.getLocation()->isAutoIndex()) {
			_logError(request, "", "directory index is forbidden");
			return (403);
		}
		LOG_DEBUG("http autoindex: \"" << _requestedFileName << "\"");
		_indexDirectory = opendir(_requestedFileName.c_str());
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
		size_t			dirPos = _requestedFileName.size() - indexLen - 1;
		char			dirChar = _requestedFileName[dirPos + (dirPos == 0)];
		struct stat		dirInfos;

		dirPos += (dirPos == 0);
		_requestedFileName[dirPos] = '\0';
		LOG_DEBUG("http index check dir: \"" << _requestedFileName << "\"");
		memset(&dirInfos, 0, sizeof(dirInfos));
		if (stat(_requestedFileName.c_str(), &dirInfos) < 0) {
			if (errno != EACCES && errno != ENOENT) {
				_logError(request, "stat()", "failed");
				return (500);
			}
			if (errno == EACCES)
				return (_loadAutoIndex(request));
			_requestedFileName[dirPos] = dirChar;
			_logError(request, "", "is not found");
			return (404);
		}
		if (S_ISDIR(dirInfos.st_mode))
			return (_loadAutoIndex(request));
		_requestedFileName[dirPos] = dirChar;
		_logError(request, "", "is not a directory");
		return (500);
	}

	int	Response::_loadIndex(Request& request)
	{
		const std::string&	index = request.getLocation()->getIndex();
		struct stat			fileInfos;

		if (index[0] == '/')
			return (_loadInternalRedirect(request, index));
		_requestedFileName += index;
		LOG_DEBUG("Open index \"" << _requestedFileName << "\"");
		memset(&fileInfos, 0, sizeof(fileInfos));
		if (stat(_requestedFileName.c_str(), &fileInfos) < 0) {
			LOG_DEBUG("stat() \"" << _requestedFileName << "\" failed ("
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
		return (_loadInternalRedirect(request, _requestedFileName));
	}

	void	Response::_closeRequestedFile()
	{
//		if (close(_requestedFileFd) < 0)
//			LOG_ERROR("Bad close() on fd=" << _requestedFileFd);
//		_requestedFileFd = -1;
		_requestedFile.clear();
		_requestedFile.close();
		if (_requestedFile.fail()) {
			LOG_ERROR("Bad close() on file " << _requestedFileName);
			_requestedFile.clear();
		}
	}

	bool	Response::_openAndStatFile(const Request& request,
										struct stat* fileInfos)
	{
		LOG_DEBUG("http filename: \"" << _requestedFileName << "\"");
//		_requestedFileFd = open(_requestedFileName.c_str(),
//				O_RDONLY | O_NONBLOCK);
		_requestedFile.open(_requestedFileName.c_str(), std::ifstream::binary);
//		if (_requestedFileFd < 0) {
		if (_requestedFile.fail()) {
			_logError(request, "open()", "failed");
			return (false);
		}
		memset(fileInfos, 0, sizeof(*fileInfos));
		if (stat(_requestedFileName.c_str(), fileInfos) < 0) {
			_logError(request, "stat()", "failed");
			_closeRequestedFile();
			return (false);
		} else if (S_ISDIR(fileInfos->st_mode))
			_closeRequestedFile();
		else
			LOG_DEBUG("http file opened: " << _requestedFileName);
//		LOG_DEBUG("http file fd: " << _requestedFileFd);
		return (true);
	}

	void	Response::_loadDirLocation(const Request& request)
	{
		_loadRelativeLocationPrefix(request);
		_location += std::string("/") + _escapeUri(request.getUri());
		if (!request.getQuery().empty())
			_location += std::string("?") + request.getQuery();
	}

	void	Response::_loadFileHeaders(const Request& request,
										const struct stat* fileInfos)
	{
		_responseCode = 200;
		_contentLength = fileInfos->st_size;
		_lastModifiedTime = fileInfos->st_mtime;
		_contentType = _getContentType(request.getExtension());
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
			LOG_DEBUG("http dir");
			_loadDirLocation(request);
			return (301);
		} else if (!S_ISREG(fileInfos.st_mode)) {
			_logError(request, "", "is not a regular file");
			_closeRequestedFile();
			return (404);
		} else if (request.getRequestMethod() == "POST") {
			_closeRequestedFile();
			return (405);	// Content of Allow header? GET if no limit_except?
		}
		_loadFileHeaders(request, &fileInfos);
		if (_contentLength > 0)
			_isFileResponse = true;
		return (0);
	}

	void	Response::_deleteDirectory(const Request& request,
										const char* dirname)
	{
		LOG_DEBUG("http delete dir: \"" << dirname << "\"");
		if (rmdir(dirname) < 0)
			_logError(request, "rmdir()", "failed", dirname);
	}

	void	Response::_deleteFile(const Request& request,
									const char* filename)
	{
		LOG_DEBUG("http delete file: \"" << filename << "\"");
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
		std::string		filepath = _requestedFileName;

		if (*request.getUri().rbegin() != '/') {
			_logError(request, "DELETE", "failed");
			return (409);
		}
		if (*filepath.rbegin() == '/')
			filepath.erase(filepath.end() - 1);
		if (!_removeDirectoryTree(request, filepath.c_str()))
			return (500);
		if (rmdir(_requestedFileName.c_str()) < 0) {
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

	int	Response::_removeRequestedFile(const Request& request)
	{
		struct stat		fileInfos;

		LOG_DEBUG("http delete filename: \"" << _requestedFileName << "\"");
		if (lstat(_requestedFileName.c_str(), &fileInfos) < 0) {
			_logError(request, "lstat()", "failed");
			if (errno == ENOTDIR)
				return (409);
			if (errno == ENOENT || errno == ENAMETOOLONG)
				return (404);
			if (errno == EACCES)
				return (403);
			return (500);
		}
		if (S_ISDIR(fileInfos.st_mode))
			return (_removeRequestedDirectory(request));
		else if (unlink(_requestedFileName.c_str()) < 0) {
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

	bool	Response::_loadFileWithAlias(Request& request)
	{
		const Location*		location = request.getLocation();
		const std::string&	locationName = location->getLocationName();
		std::string			aliasUri = request.getUri();

		if (location->getAlias().empty())
			return (false);
		aliasUri.replace(0, locationName.size(), location->getAlias());
		if (aliasUri[0] == '/')
			_requestedFileName = aliasUri;
		else {
			if (*_requestedFileName.rbegin() != '/')
				_requestedFileName += '/';
			_requestedFileName += aliasUri;
		}
		return (true);
	}

	int	Response::_loadRequestedFile(Request& request)
	{
		if (!_loadFileWithAlias(request)) {
			if (request.getLocation()->getRoot()[0] == '/')
				_requestedFileName = request.getLocation()->getRoot();
			else {
				if (*_requestedFileName.rbegin() != '/')
					_requestedFileName += '/';
				_requestedFileName += request.getLocation()->getRoot();
			}
			if (*_requestedFileName.rbegin() == '/')
				_requestedFileName.erase(_requestedFileName.end() - 1);
			_requestedFileName += request.getUri();
		}
		if (request.getRequestMethod() == "DELETE")
			return (_removeRequestedFile(request));
		if (*(_requestedFileName.end() - 1) == '/')
			return (_loadIndex(request));
		return (_openRequestedFile(request));
	}

	void	Response::prepareResponse(Request& request)
	{
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
		_loadHeaders();
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
				if (request.getRequestMethod() != "GET")
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
		_loadHeaders();
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
		_loadHeaders();
		_responseBuffer += *specialBody;
		_isResponseReady = true;
	}

	void	Response::clearResponse(const Request& request,
									int responseCodeToKeep)
	{
		_responseBuffer.clear();
		_requestedFileName = XSTR(WEBSERV_ROOT);
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
