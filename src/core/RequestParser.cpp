#include <stddef.h>		// size_t
#include <stdint.h>		// int64_t

#include <cctype>		// isalnum, isdigit, isupper, toupper
#include <cstdlib>		// atof, strtoll

#include <algorithm>	// search
#include <limits>		// numeric_limits
#include <utility>		// make_pair

#include "core/Request.hpp"
#include "utils/log.hpp"
#include "utils/utils.hpp"

#define REQUEST_LINE_MAX	8000

namespace webserv
{

	size_t	Request::_parseMethod(const char* str)
	{
		size_t	i = 0;

		while (std::isupper(str[i]) || str[i] == '_' || str[i] == '-')
			++i;
		if (i == 0 || str[i] != ' ') {
			_logError("Client sent invalid method in request line");
			_errorCode = 400;
			return (std::string::npos);
		}
		_method.assign(str, i);
		++i;
		while (str[i] == ' ')
			++i;
		return (i);
	}

	size_t	Request::_parseOriginUri(const char* str)
	{
		size_t	i = checkUriPathAbs(str, "? ");

		if (i != std::string::npos && str[i])
			_uri.assign(str, i);
		if (i != std::string::npos && str[i] == '?') {
			str += i;
			i = checkUriQuery(++str, " ");
			if (i != std::string::npos)
				_query.assign(str, i);
		}
		if (i == std::string::npos || str[i] != ' ') {
			_logError("Client sent invalid URI in request line");
			_errorCode = 400;
			return (std::string::npos);
		}
		return (i);
	}

	size_t	Request::_parseUriHost(const char* str)
	{
		size_t	i;
		size_t	j = 0;

		i = checkUriHost(str);
		if (i == std::string::npos || !str[i] || i == 0) {
			_logError("Client sent invalid host in request line");
			_errorCode = 400;
			return (std::string::npos);
		}
		if (str[i] == ':') {
			++i;
			j = checkUriPort(str + i);
			if (j == std::string::npos || !str[i + j]) {
				_logError("Client sent invalid port in request line");
				_errorCode = 400;
				return (std::string::npos);
			}
		}
		_host.assign(str, i + j);
		return (i + j);
	}

	size_t	Request::_parseUriPath(const char* str)
	{
		size_t	i = 0;

		if (*str == '?' || *str == ' ')
			_uri = "/";
		else {
			i = checkUriPathAbEmpty(str);
			if (i == std::string::npos || !str[i]) {
				_logError("Client sent invalid URI in request line");
				_errorCode = 400;
				return (std::string::npos);
			}
			_uri.assign(str, i);
		}
		return (i);
	}

	size_t	Request::_parseUriInfos(const char* str)
	{
		size_t	i = 0;
		size_t	j = 0;

		while (1) {
			if (*str == '?') {
				i = checkUriQuery(str + 1);
				if (i == std::string::npos || !str[i + 1])
					break ;
				_query.assign(str + 1, i);
				i += 1;
				str += i;
			}
			if (*str == '#') {
				j = checkUriFragment(str + 1);
				if (j == std::string::npos || !str[j + 1])
					break ;
				j += 1;
			}
			return (i + j);
		}
		_logError("Client sent invalid URI in request line");
		_errorCode = 400;
		return (std::string::npos);
	}

	size_t	Request::_parseAbsoluteUri(const char* str)
	{
		const char*	start = str;
		size_t		i;

		i = checkUriScheme(str);
		if (i == std::string::npos || str[i + 1] != '/' || str[i + 2] != '/') {
			_logError("Client sent invalid URI in request line");
			_errorCode = 400;
			return (std::string::npos);
		}
		str += i + 3;
		i = _parseUriHost(str);
		if (i == std::string::npos)
			return (std::string::npos);
		str += i;
		i = _parseUriPath(str);
		if (i == std::string::npos)
			return (std::string::npos);
		str += i;
		i = _parseUriInfos(str);
		if (i == std::string::npos)
			return (std::string::npos);
		str += i;
		return (str - start);
	}

	size_t	Request::_handleDotSegment(size_t i, std::string& uri) const
	{
		const char*	str = _uri.c_str();
		size_t		j;

		if ((i == 0 || str[i - 1] == '/') && str[i + 1] == '/')
			return (i + 1 + (i != 0));
		if ((i == 0 || str[i - 1] == '/')
				&& str[i + 1] == '.' && str[i + 2] == '/') {
			j = uri.find_last_of('/');
			if (j == std::string::npos || j == 0 || uri[0] != '/')
				return (std::string::npos);
			uri.erase(uri.find_last_of('/', (j - 1)) + 1);
			return (i + 2 + (i != 0));
		}
		uri += '.';
		return (i + 1);
	}

	size_t	Request::_unescapePctChar(size_t i, std::string& uri) const
	{
		static const std::string	hex = "0123456789ABCDEF";
		const char*					str = _uri.c_str();
		char						c;

		if (hex.find(std::toupper(str[i + 1])) == std::string::npos
				|| hex.find(std::toupper(str[i + 2])) == std::string::npos)
			return (std::string::npos);
		c = hex.find(std::toupper(str[i + 1])) * 16;
		c += hex.find(std::toupper(str[i + 2]));
		uri += c;
		return (i + 3);
	}

	bool	Request::_logUri()
	{
		LOG_DEBUG("HTTP uri: " << _uri);
		LOG_DEBUG("HTTP query: " << _query);
		_extension = getFileExtension(_uri);
		LOG_DEBUG("HTTP extension: " << _extension);
		return (true);
	}

	bool	Request::_decodeUri()
	{
		const char*		str = _uri.c_str();
		size_t			i = 0;
		std::string		uri;

		while (str[i]) {
			if (str[i] == '.') {
				i = _handleDotSegment(i, uri);
				if (i == std::string::npos)
					break ;
			} else if (str[i] == '%') {
				i = _unescapePctChar(i, uri);
				if (i == std::string::npos)
					break ;
			} else if (str[i] == '/' && !uri.empty() && *uri.rbegin() == '/')
				++i;
			else
				uri += str[i++];
		}
		if (!str[i] && i != 0) {
			_uri = uri;
			return (_logUri());
		}
		_logError("Client sent invalid URI in request line");
		_errorCode = 400;
		return (false);
	}

	size_t	Request::_parseHttpVersion(const char* str)
	{
		size_t	i = 0;

		while (str[i] == ' ')
			++i;
		if (!str[i])
			_httpVersion = "0.9";
		else if (std::string(&str[i], 5) != "HTTP/" || !std::isdigit(str[i + 5])
				|| str[i + 6] != '.' || !std::isdigit(str[i + 7]))
			_errorCode = 400;
		if (!_errorCode && _httpVersion.empty())
			_httpVersion = std::string(str + i + 5, 3);
		if (!_errorCode && _httpVersion != "1.1" && _httpVersion != "1.0")
			_errorCode = 505;
		if (!_errorCode && _httpVersion == "1.0")
			_isKeepAlive = false;
		i += 7;
		while (!_errorCode && str[++i]) {
			if (str[i] != ' ')
				_errorCode = 400;
		}
		if (_errorCode == 505)
			_logError("Client sent unsupported HTTP version in request line");
		if (_errorCode == 400)
			_logError("Client sent invalid HTTP version in request line");
		return (_errorCode == 0 ? i : std::string::npos);
	}

	bool	Request::_parseRequestLine(const std::string& line)
	{
		const char*	str = line.c_str();
		size_t		i;

		if (line.empty() || line == "\n" || line.size() > REQUEST_LINE_MAX) {
			if ((line.empty() && _buffer.size() - _bufferIndex
					> REQUEST_LINE_MAX) || line.size() > REQUEST_LINE_MAX) {
				_logError("Client sent a request line too long");
				_errorCode = 414;
			}
			return (false);
		}
		_requestLine = line;
		LOG_DEBUG("HTTP process request line: " << strHexDump(_requestLine));
		i = _parseMethod(str);
		if (i == std::string::npos)
			return (false);
		str += i;
		i = (*str == '/' ? _parseOriginUri(str) : _parseAbsoluteUri(str));
		if (i == std::string::npos || !_decodeUri())
			return (false);
		str += i;
		if (_parseHttpVersion(str) == std::string::npos)
			return (false);
		return (true);
	}

	bool	Request::_isHeaderName(const char c) const
	{
		if (std::isalnum(c) || c == '-')
			return (true);
		return (false);
	}

	bool	Request::_loadHeaderValue(const std::string& line,
										std::string::const_iterator c,
										std::string& fieldValue)
	{
		std::string::const_iterator	startValue = line.end();

		while (*c == ' ' && c != line.end())
			++c;
		while (c != line.end()) {
			if (!*c || *c == '\r') {
				_logError("Client sent an invalid header line");
				_errorCode = 400;
				return (false);
			} else if (startValue == line.end())
				startValue = c;
			++c;
		}
		if (startValue != line.end()) {
			while (*(c - 1) == ' ')
				--c;
			fieldValue.assign(startValue, c);
		}
		return (true);
	}

	bool	Request::_loadHeaderFields(const std::string& line,
										std::string& fieldName,
										std::string& fieldValue)
	{
		// NOTE: NGINX is way more strict than the RFC regarding the
		// headers parsing, in order to stay safe
		// (see https://trac.nginx.org/nginx/ticket/629#comment:4)

		std::string::const_iterator	c = line.begin();

		while (_isHeaderName(*c) && c != line.end())
			++c;
		if (c != line.end() && *c > ' ' && *c <= '~' && *c != ':') {
			LOG_INFO("Client sent an invalid header line");
			return (true);
		}
		if (c != line.end() && (*c <= ' ' || *c > '~' || line[0] == ':')) {
			_logError("Client sent an invalid header line");
			_errorCode = 400;
			return (false);
		}
		fieldName.assign(line.begin(), c);
		if (c == line.end() || (c + 1) == line.end())
			return (true);
		++c;
		return (_loadHeaderValue(line, c, fieldValue));
	}

	bool	Request::_recordHostHeader(const std::string& fieldValue)
	{
		if (_headers.count("Host")) {
			_logError("Client sent duplicate host header");
			_errorCode = 400;
			return (false);
		}
		if (checkUriHost(fieldValue.c_str()) == std::string::npos) {
			_logError("Client sent invalid host header");
			_errorCode = 400;
			return (false);
		}
		if (_host.empty())
			_host = fieldValue;
		_headers["Host"] = _host;
		return (true);
	}

	bool	Request::_recordConnectionHeader(const std::string& fieldValue)
	{
		const std::string	keepAlive = "keep-alive";
		const std::string	close = "close";

		_headers["Connection"] = fieldValue;
		if (std::search(fieldValue.begin(), fieldValue.end(), close.begin(),
					close.end(), ft_charcmp_icase) != fieldValue.end())
			_isKeepAlive = false;
		else if (std::search(fieldValue.begin(), fieldValue.end(),
					keepAlive.begin(), keepAlive.end(),
					ft_charcmp_icase) != fieldValue.end())
			_isKeepAlive = true;
		return (true);
	}

	bool	Request::_recordAcceptLanguageHeader(const std::string& fieldValue)
	{
		std::vector<std::string>	languages;
		std::vector<std::string>	value;
		size_t						i;
		double						weight;

		if (fieldValue.empty())
			return (true);
		languages = ft_string_split(ft_string_remove(fieldValue, ' '), ",");
		for (std::vector<std::string>::iterator it = languages.begin();
				it != languages.end(); ++it) {
			value = ft_string_split(*it, ";");
			if (value.size() == 1)
				_languages.insert(std::make_pair(1.0, value[0]));
			else {
				i = value[1].find("q=");
				weight = 0.0;
				if (i != std::string::npos)
					weight = std::atof(value[1].c_str() + i + 2);
				_languages.insert(std::make_pair(weight, value[0]));
			}
		}
		return (true);
	}

	bool	Request::_recordGenericHeader(const std::string& fieldName,
											const std::string& fieldValue,
											bool mustBeUnique)
	{
		if (mustBeUnique && _headers.count(fieldName)) {
			_logError("Client sent duplicate header line");
			LOG_DEBUG("\"" << fieldName << ": " << fieldValue << "\""
					<< ", previous value: \"" << fieldName << ": "
					<< _headers[fieldName] << "\"");
			_errorCode = 400;
			return (false);
		}
		_headers[fieldName] = fieldValue;
		return (true);
	}

	bool	Request::_recordOtherHeader(const std::string& fieldName,
										const std::string& fieldValue)
	{
		if (ft_strcmp_icase(fieldName, "User-Agent")
				|| ft_strcmp_icase(fieldName, "Referer")
				|| ft_strcmp_icase(fieldName, "Range")
				|| ft_strcmp_icase(fieldName, "TE")
				|| ft_strcmp_icase(fieldName, "Upgrade")
				|| ft_strcmp_icase(fieldName, "Accept-Encoding")
				|| ft_strcmp_icase(fieldName, "Via")
				|| ft_strcmp_icase(fieldName, "Keep-Alive")
				|| ft_strcmp_icase(fieldName, "X-Forwarded-For")
				|| ft_strcmp_icase(fieldName, "X-Real-IP")
				|| fieldName.find("X-") == 0
				|| ft_strcmp_icase(fieldName, "Accept")
				|| ft_strcmp_icase(fieldName, "Accept-Language")
				|| ft_strcmp_icase(fieldName, "Date")
				|| ft_strcmp_icase(fieldName, "Cookie"))
			return (_recordGenericHeader(fieldName, fieldValue));
		return (true);
	}

	bool	Request::_recordHeader(const std::string& fieldName,
									const std::string& fieldValue)
	{
		if (ft_strcmp_icase(fieldName, "Host"))
			return (_recordHostHeader(fieldValue));
		if (ft_strcmp_icase(fieldName, "Connection"))
			return (_recordConnectionHeader(fieldValue));
		if (ft_strcmp_icase(fieldName, "Accept-Language"))
			return (_recordAcceptLanguageHeader(fieldValue));
		if (ft_strcmp_icase(fieldName, "If-Modified-Since")
				|| ft_strcmp_icase(fieldName, "If-Unmodified-Since")
				|| ft_strcmp_icase(fieldName, "If-Match")
				|| ft_strcmp_icase(fieldName, "If-None-Match")
				|| ft_strcmp_icase(fieldName, "Content-Length")
				|| ft_strcmp_icase(fieldName, "Content-Range")
				|| ft_strcmp_icase(fieldName, "If-Range")
				|| ft_strcmp_icase(fieldName, "Transfer-Encoding")
				|| ft_strcmp_icase(fieldName, "Expect")
				|| ft_strcmp_icase(fieldName, "Authorization"))
			return (_recordGenericHeader(fieldName, fieldValue, true));
		return (_recordOtherHeader(fieldName, fieldValue));
	}

	bool	Request::_parseHeaderLine(const std::string& line)
	{
		std::string		fieldName;
		std::string		fieldValue;

		if (line.empty() || line.size() > REQUEST_LINE_MAX) {
			if ((line.empty() && _buffer.size() - _bufferIndex
					> REQUEST_LINE_MAX) || line.size() > REQUEST_LINE_MAX) {
				_logError("Client sent a header line too long");
				_errorCode = 431;
			}
			return (false);
		}
		if (line == "\n") {
			LOG_DEBUG("HTTP header done");
			_hasReceivedHeaders = true;
			return (true);
		}
		LOG_DEBUG("HTTP process header line: " << strHexDump(line));
		if (!_loadHeaderFields(line, fieldName, fieldValue))
			return (false);
		if (!_recordHeader(fieldName, fieldValue))
			return (false);
		LOG_DEBUG("HTTP header: \"" << fieldName << ": " << fieldValue << "\"");
		return (true);
	}

	bool	Request::_checkHost()
	{
		size_t	i;

		if (_headers.count("Host") == 0 && _httpVersion == "1.1") {
			_logError("Client sent HTTP/1.1 request without \"Host\" header");
			_errorCode = 400;
			return (false);
		}
		i = _host.find(':');
		if (i == std::string::npos)
			i = _host.size();
		i -= (i != 0 && _host[i - 1] == '.');
		_host.erase(i);
		return (true);
	}

	bool	Request::_checkContentLength()
	{
		header_map::iterator	contentLength = _headers.find("Content-Length");
		char*					endPtr;

		if (contentLength == _headers.end())
			return (true);
		if (!contentLength->second.empty()) {
			errno = 0;
			_contentLength = std::strtoll(contentLength->second.c_str(),
					&endPtr, 10);
		}
		if (contentLength->second.empty() || _contentLength < 0
				|| _contentLength == std::numeric_limits<int64_t>::max()
				|| errno != 0 || *endPtr) {
			_logError("Client sent invalid \"Content-Length\" header");
			_errorCode = 400;
			return (false);
		}
		_bodySize = _contentLength;
		return (true);
	}

	bool	Request::_checkTransferEncoding()
	{
		header_map::iterator	it = _headers.find("Transfer-Encoding");
		const std::string&		transferEncoding = (it != _headers.end() ?
															it->second : "");

		if (transferEncoding.empty())
			return (true);
		if (_httpVersion == "1.0") {
			_logError("Client sent HTTP/1.0 request with Transfer-Encoding");
			_errorCode = 400;
		} else if (ft_strcmp_icase(transferEncoding, "chunked")) {
			_isChunkedRequest = true;
			if (_contentLength >= 0) {
				_logError("Client sent \"Content-Length\" and "
						"\"Transfer-Encoding\" headers");
				_errorCode = 400;
			}
		} else {
			_logError("Client sent unknown \"Transfer-Encoding\"");
			LOG_DEBUG("\"Transfer-Encoding: " << transferEncoding << "\"");
			_errorCode = 501;
		}
		return (_errorCode ? false : true);
	}

	bool	Request::_parseStartAndFieldLines()
	{
		if (_requestLine.empty() && !_parseRequestLine(_readLine(false)))
			return (false);
		while (!_hasReceivedHeaders && _bufferIndex < _buffer.size()) {
			if (!_parseHeaderLine(_readLine()))
				break ;
		}
		if (_hasReceivedHeaders) {
			if (!_checkHost() || !_checkContentLength()
					|| !_checkTransferEncoding())
				return (false);
			_clearBuffer();
			return (true);
		}
		return (false);
	}

	bool	Request::_checkChunkExt(const std::string& line, size_t i)
	{
		if (i < line.size() && line[i] != ';'
				&& line[i] != ' ' && line[i] != '\t') {
			_logError("Client sent invalid chunk extension");
			_errorCode = 400;
			return (false);
		}
		while (++i < line.size()) {
			if (line[i] == '\r') {
				_logError("Client sent invalid chunk extension");
				_errorCode = 400;
				return (false);
			}
		}
		return (true);
	}

	size_t	Request::_convertChunkSize(const std::string& line)
	{
		static const std::string	hex = "0123456789ABCDEF";
		size_t						i = 0;

		while (i < line.size() && hex.find(std::toupper(line[i])) < 16)
			++i;
		if (i > 0) {
			errno = 0;
			_bodySize = std::strtoll(line.c_str(), 0, 16);
		}
		if (i == 0 || errno != 0
				|| _bodySize == std::numeric_limits<int64_t>::max()) {
			_logError("Client sent invalid chunk size");
			_errorCode = 400;
			return (std::string::npos);
		}
		return (i);
	}

	bool	Request::_parseChunkSize(const std::string& line)
	{
		size_t	i;

		if (line.empty() || line.size() > REQUEST_LINE_MAX) {
			if ((line.empty() && _buffer.size() - _bufferIndex
					> REQUEST_LINE_MAX) || line.size() > REQUEST_LINE_MAX) {
				_logError("Client sent a chunk size line too long");
				_errorCode = 400;
			}
			return (false);
		}
		LOG_DEBUG("HTTP process chunk size line: " << strHexDump(line));
		i = _convertChunkSize(line);
		if (i == std::string::npos || !_checkChunkExt(line, i))
			return (false);
		if (_contentLength < 0)
			_contentLength = 0;
		_contentLength += _bodySize;
		return (true);
	}

	bool	Request::_parseChunkTrailer(const std::string& line)
	{
		size_t	i = 0;

		if (line == "\n") {
			LOG_DEBUG("HTTP chunk trailer done");
			_headers.erase("Transfer-Encoding");
			_closeTmpFile();
			_isChunkedRequest = false;
			_isTerminatedRequest = true;
			return (true);
		}
		LOG_DEBUG("HTTP process chunk trailer line: " << strHexDump(line));
		while (i < line.size()) {
			if (line[i] == '\r') {
				_logError("Client sent invalid chunk trailer");
				_errorCode = 400;
				return (false);
			}
			++i;
		}
		return (false);
	}

	bool	Request::_loadChunk()
	{
		int64_t	inBuffer = (_buffer.size() >= _bufferIndex ?
							_buffer.size() - _bufferIndex : 0);
		int64_t	toLoad = (_bodySize > inBuffer ? inBuffer : _bodySize);
		size_t	i = 0;

		if (toLoad == _bodySize) {
			i = _bufferIndex + toLoad;
			if (i >= _buffer.size()
					|| (_buffer[i] == '\r' && i + 1 == _buffer.size()))
				return (false);
			if (_buffer[i] != '\n' && std::string(&_buffer[i], 2) != "\r\n") {
				_logError("Client sent invalid chunked body");
				_errorCode = 400;
				return (false);
			}
			i += (1 + (_buffer[i] == '\r'));
		}
		if (!_writeBodyTmpFile(toLoad))
			return (false);
		_bufferIndex = (i != 0 ? i : _bufferIndex + toLoad);
		if (_bufferIndex >= _buffer.size())
			_clearBuffer();
		if ((_bodySize -= toLoad) == 0)
			_bodySize = -1;
		return (true);
	}

	int	Request::_parseChunkedRequest()
	{
		if (_method != "POST" && _method != "PUT") {
			_discardBody();
			return (0);
		}
		LOG_DEBUG("HTTP load request chunked body");
		if (_tmpFilename.empty() && !_createBodyTmpFile())
			return (_errorCode);
		if (_bufferIndex >= _buffer.size())
			return (0);
		if (_bodySize < 0) {
			if (!_parseChunkSize(_readLine()) || !_checkMaxBodySize())
				return (_errorCode);
		}
		if (_bodySize == 0 && !_parseChunkTrailer(_readLine()))
			return (_errorCode);
		if (_bodySize > 0 && !_loadChunk())
			return (_errorCode);
		if (_bufferIndex < _buffer.size())
			return (_parseChunkedRequest());
		return (0);
	}

	bool	Request::_loadBody()
	{
		int64_t	toLoad = _bodySize;

		if (_method != "POST" && _method != "PUT") {
			_discardBody();
			return (true);
		}
		LOG_DEBUG("HTTP load request content length body");
		if (_tmpFilename.empty() && !_createBodyTmpFile())
			return (false);
		if (toLoad > 0 && _bufferIndex >= _buffer.size())
			return (true);
		if (static_cast<size_t>(toLoad) > _buffer.size() - _bufferIndex)
			toLoad = _buffer.size() - _bufferIndex;
		if (!_writeBodyTmpFile(toLoad))
			return (false);
		_bufferIndex += toLoad;
		_clearBuffer();
		_bodySize -= toLoad;
		if (_bodySize == 0)
			_closeTmpFile();
		return (true);
	}

	int	Request::parseRequest(const char* recvBuffer,
								const server_configs& serverConfigs)
	{
		// TO DO: if client sends more chunks after error, and error not
		// disconnecting, continue discarding until done (all chunks received)
		// or until receiving a request line?
		// Or just make sure to leave time to send first error response???

		if (recvBuffer)
			_buffer += recvBuffer;
		if ((!recvBuffer || !recvBuffer[0]) && _buffer.empty())
			return (0);
		if (!_hasReceivedHeaders && !_parseStartAndFieldLines())
			return (_errorCode);
		if (!_serverConfig && (!_loadServerConfig(serverConfigs)
					|| !_checkConfig())) {
			_discardBody();
			return (_errorCode);
		}
		if (_isChunkedRequest)
			return (_parseChunkedRequest());
		if (_bodySize >= 0 && !_loadBody()) {
			_discardBody();
			return (_errorCode);
		}
		if (_bodySize <= 0)
			_isTerminatedRequest = true;
		return (0);
	}

}	//namespace webserv
