#include <arpa/inet.h>	// ntohs
#include <stddef.h>		// size_t
#include <stdint.h>		// int64_t
#include <unistd.h>		// unlink

#include <cerrno>		// errno
#include <cstring>		// strerror

#include <algorithm>	// search
#include <ios>			// ios::out/binary
#include <sstream>		// ostringstream

#include "core/Request.hpp"
#include "utils/log.hpp"
#include "utils/utils.hpp"

namespace	webserv
{

	/**************************************************************************/
	/*                       CONSTRUCTORS / DESTRUCTORS                       */
	/**************************************************************************/

	Request::Request(const AcceptSocket& clientSocket):
												_clientSocket(clientSocket),
												_serverConfig(0),
												_location(0),
												_bufferIndex(0),
												_errorCode(0),
												_contentLength(-1),
												_bodySize(-1),
												_isKeepAlive(true),
												_isChunkedRequest(false),
												_hasReceivedHeaders(false),
												_isTerminatedRequest(false),
												_isInternalRedirect(false)
	{
		LOG_INFO("New Request instance");
	}

	Request::Request(const Request& src): _clientSocket(src._clientSocket),
											_serverConfig(0),
											_location(0),
											_bufferIndex(0),
											_errorCode(0),
											_contentLength(-1),
											_bodySize(-1),
											_isKeepAlive(true),
											_isChunkedRequest(false),
											_hasReceivedHeaders(false),
											_isTerminatedRequest(false),
											_isInternalRedirect(false)
	{
		// NOTE: Except at Client creation (inserted in the client list),
		// the Request should not be copied

		LOG_INFO("Request copied");
	}

	Request::~Request()
	{
		if (_tmpFile.is_open())
			_closeTmpFile();
		if (!_tmpFilename.empty())
			_deleteTmpFile();
	}

	/**************************************************************************/
	/*                            MEMBER FUNCTIONS                            */
	/**************************************************************************/

	const std::string&	Request::getServerName() const
	{
		static const std::string					emptyString("");
		ServerConfig::hostname_set::const_iterator	serverName;

		if (_serverConfig && !_serverConfig->getServerNames().empty()) {
			serverName = _serverConfig->getServerNames().find(_host);
			if (serverName != _serverConfig->getServerNames().end())
				return (*serverName);
			return (*_serverConfig->getServerNames().begin());
		}
		return (emptyString);
	}

	const std::string&	Request::getContentType() const
	{
		static const std::string	emptyString("");
		header_map::const_iterator	contentType = _headers.find("Content-Type");

		if (contentType != _headers.end())
			return (contentType->second);
		return (emptyString);
	}

	bool	Request::isKeepAlive() const
	{
		if (!_isKeepAlive)
			return (false);
		if (_errorCode == 400 || _errorCode == 408
				|| _errorCode == 413 || _errorCode == 414
				|| _errorCode == 495 || _errorCode == 496 || _errorCode == 497
				|| _errorCode == 500 || _errorCode == 501)
			return (false);
		return (true);
	}

	void	Request::setRequestMethod(const std::string& method)
	{
		_requestMethod = method;
	}

	void	Request::_parseInternalTarget(const std::string& redirectTo)
	{
		size_t	query = redirectTo.find('?');

		_uri = redirectTo.substr(0, query);
		if (query != std::string::npos)
			_query = redirectTo.substr(query + 1);
		else
			_query.clear();
		_extension = getFileExtension(_uri);
	}

	int	Request::loadInternalRedirect(const std::string& redirectTo)
	{
		// NOTE: During internal redirections, NGINX does not check the URI
		// validity (!!!) and allows it to go up like "/../../../[file]"

		_isInternalRedirect = true;
		if (redirectTo.empty() || redirectTo[0] != '/') {
			LOG_DEBUG("Incorrect internal redirect: \"" << redirectTo << "\"");
			return (500);
		}
		_parseInternalTarget(redirectTo);
		LOG_DEBUG("Internal redirect: \"" << _uri << "?" << _query << "\"");
		if (!_loadLocation(*_serverConfig))
			return (500);
		if (!_checkConfig())
			return (_errorCode);
		return (0);
	}

	void	Request::_logError(const char* errorAt) const
	{
		std::ostringstream	msg;

		msg << errorAt << ", client: " << _clientSocket.getIpAddr()
			<< ":" << _clientSocket.getPort();
		if (_serverConfig) {
			msg << ", server: "
				<< ft_inet_ntoa(_serverConfig->getListenPair().sin_addr)
				<< ":" << ntohs(_serverConfig->getListenPair().sin_port)
				<< " (\"" << getServerName() << "\")";
		}
		msg << ", request: \"" << _requestLine
			<< "\", host: \"" << _host << "\""
			<< ", body size: " << _contentLength;
		LOG_ERROR(msg.str().c_str());
	}

	void	Request::_printRequestInfo() const
	{
		LOG_INFO("Request Line: " << _requestLine);
		LOG_INFO("Request Method: " << _requestMethod);
		LOG_INFO("Request URI: " << _uri);
		LOG_INFO("Request Query: " << _query);
		LOG_INFO("Request Extension: " << _extension);
		LOG_INFO("Request HTTP Version: " << _httpVersion);
		LOG_INFO("Request Error Code: " << _errorCode);
		LOG_INFO("Request Host: " << _host);
		LOG_INFO("Request Content-Length: " << _contentLength);
		LOG_INFO("Request Body Size: " << _bodySize);
		LOG_INFO("Request Keep-Alive: " << _isKeepAlive);
		LOG_INFO("Request Chunked: " << _isChunkedRequest);
		LOG_INFO("Request Has Received Headers: " << _hasReceivedHeaders);
		LOG_INFO("Request Recorded Headers: ");
		for (header_map::const_iterator it = _headers.begin();
				it != _headers.end(); ++it) {
			LOG_INFO("\"" << it->first << ": " << it->second << "\"");
		}
		LOG_INFO("Request Is Terminated: " << _isTerminatedRequest);
		LOG_INFO("Request Is Internal Redirect: " << _isInternalRedirect);
	}

	bool	Request::_loadExtensionLocation(const ServerConfig& serverConfig)
	{
		_location_map::const_iterator		extLocation;

		extLocation = serverConfig.getLocations().lower_bound("*.~");
		while (extLocation != serverConfig.getLocations().end()
				&& extLocation->first[0] == '*'
				&& extLocation->first.size() > 2) {
			if (std::search(_uri.rbegin(), _uri.rend(),
						extLocation->first.rbegin(),
						extLocation->first.rend() - 1,
						&ft_charcmp_icase) == _uri.rbegin()) {
				_location = &(extLocation->second);
				LOG_DEBUG("Using location: \"" << extLocation->first << "\"");
				return (true);
			}
			LOG_DEBUG("Test location: \"" << extLocation->first << "\"");
			++extLocation;
		}
		return (false);
	}

	bool	Request::_loadExtensionLocation(const Location& location)
	{
		_location_map::const_iterator		extLocation;

		extLocation = location.getNestedLocations().lower_bound("*.~");
		while (extLocation != location.getNestedLocations().end()
				&& extLocation->first[0] == '*'
				&& extLocation->first.size() > 2) {
			if (std::search(_uri.rbegin(), _uri.rend(),
						extLocation->first.rbegin(),
						extLocation->first.rend() - 1,
						&ft_charcmp_icase) == _uri.rbegin()) {
				_location = &(extLocation->second);
				LOG_DEBUG("Using location: \"" << extLocation->first << "\"");
				return (true);
			}
			LOG_DEBUG("Test location: \"" << extLocation->first << "\"");
			++extLocation;
		}
		return (false);
	}

	bool	Request::_loadLocation(const ServerConfig& serverConfig)
	{
		_location_map::const_iterator		location;

		if (_loadExtensionLocation(serverConfig))
			return (true);
		location = serverConfig.getLocations().lower_bound(_uri);
		while (location != serverConfig.getLocations().end()) {
			if (location->first == "" || std::search(_uri.begin(), _uri.end(),
						location->first.begin(), location->first.end(),
						&ft_charcmp_icase) == _uri.begin()) {
				_location = &(location->second);
				if (_loadExtensionLocation(*_location)) {
					LOG_DEBUG("Inside location: \"" << location->first << "\"");
				} else {
					LOG_DEBUG("Using location: \"" << location->first << "\"");
				}
				return (true);
			}
			if (location->first[0] != '*')
				LOG_DEBUG("Test location: \"" << location->first << "\"");
			++location;
		}
		LOG_ERROR("No suitable location found for uri: \"" << _uri << "\"");
		_errorCode = 500;
		return (false);
	}

	bool	Request::_loadServerConfig(const server_configs& serverConfigs)
	{
		server_configs::const_iterator				config;
		ServerConfig::hostname_set::const_iterator	name;

		config = serverConfigs.begin();
		while (config != serverConfigs.end()) {
			name = config->getServerNames().begin();
			while (name != config->getServerNames().end()) {
				if (ft_strcmp_icase(_host, *name) == true) {
					_serverConfig = &(*config);
					LOG_DEBUG("Using server: \"" << *name << "\" (on \""
						<< ft_inet_ntoa(_serverConfig->getListenPair().sin_addr)
						<< ":" << ntohs(_serverConfig->getListenPair().sin_port)
						<< "\")");
					return (_loadLocation(*_serverConfig));
				}
				++name;
			}
			++config;
		}
		_serverConfig = &(serverConfigs[0]);
		LOG_DEBUG("Using default server (on \""
				<< ft_inet_ntoa(_serverConfig->getListenPair().sin_addr) << ":"
				<< ntohs(_serverConfig->getListenPair().sin_port) << "\")");
		return (_loadLocation(*_serverConfig));
	}

	bool	Request::_checkMaxBodySize()
	{
		LOG_DEBUG("Content-Length: " << _contentLength << ", max: "
				<< _location->getMaxBodySize());
		if (_contentLength > 0
				&& _contentLength > _location->getMaxBodySize()) {
			_logError("Client intended to send too large body");
			_errorCode = 413;
			return (false);
		}
		return (true);
	}

	bool	Request::_checkMethod()
	{
		if (_requestMethod == "CONNECT" || _requestMethod == "OPTIONS"
				|| _requestMethod == "TRACE" || _requestMethod == "PATCH")
			_errorCode = 405;
		if (_requestMethod != "GET" && _requestMethod != "HEAD"
				&& _requestMethod != "POST" && _requestMethod != "PUT"
				&& _requestMethod != "DELETE")
			_errorCode = 501;
		if (!_location->getLimitExcept().empty()
				&& !_location->getLimitExcept().count(_requestMethod)) {
			_logError("Access forbidden by rule");
			_errorCode = 403;
		}
		if (_errorCode)
			return (false);
		return (true);
	}

	bool	Request::_checkReturn()
	{
		if (_location->getReturnPair().first >= 0) {
			_isTerminatedRequest = true;
			return (false);
		}
		return (true);
	}

	bool	Request::_checkConfig()
	{
		// TO DO: In case of PUT/POST, before loading the request body,
		// check if the POST request will be processed, to avoid a potential
		// blocking and consuming of time and resources while loading a large
		// file that would be dropped because of an error right afterwards ?
		// (i.e. more than the return directive, check if there is a CGI,
		// or after the root/alias translation, there is a filename that is not
		// a directory, and for POST, that doesn't already exist)

		if (!_checkMaxBodySize())
			return (false);
		if (!_checkMethod())
			return (false);
		if (!_checkReturn())
			return (false);
		return (true);
	}

	void	Request::_discardChunkedTrailer()
	{
		int			savedErrorCode = _errorCode;
		std::string	line;

		while (1) {
			line = _readLine();
			if (_parseChunkTrailer(line))
				return ;
			if (line.empty() || _errorCode != 0) {
				if (line.empty())
					_clearBuffer();
				else
					_buffer.clear();
				_errorCode = savedErrorCode;
				return ;
			}
		}
	}

	void	Request::_discardChunkedBody()
	{
		int		savedErrorCode = _errorCode;

		LOG_DEBUG("HTTP discard chunked body");
		while (1) {
			if (!_parseChunkSize(_readLine()))
				break ;
			if (_bodySize == 0)
				return (_discardChunkedTrailer());
			if (static_cast<size_t>(_bodySize) >= _buffer.size() - _bufferIndex
					|| (_buffer[_bufferIndex + _bodySize] != '\r'
						&& _buffer[_bufferIndex + _bodySize] != '\n')
					|| (*(_buffer.c_str() + _bufferIndex + _bodySize) == '\r'
						&& _buffer[_bufferIndex + _bodySize + 1] != '\n')) {
				_errorCode = 400;
				break ;
			}
			_bufferIndex += _bodySize;
			_bodySize = -1;
		}
		if (_errorCode)
			_buffer.clear();
		else
			_clearBuffer();
		_errorCode = savedErrorCode;
	}

	void	Request::_discardBody()
	{
		if (!isKeepAlive()) {
			_buffer.clear();
			return ;
		}
		_isTerminatedRequest = true;
		if (_isChunkedRequest) {
			_isChunkedRequest = false;
			return (_discardChunkedBody());
		}
		LOG_DEBUG("HTTP discard body");
		if (_bodySize > 0) {
			if (_bufferIndex < _buffer.size())
				_bufferIndex += _bodySize;
			_clearBuffer();
		}
	}

	void	Request::_clearBuffer()
	{
		if (_bufferIndex >= _buffer.size())
			_buffer.clear();
		else
			_buffer.erase(0, _bufferIndex);
		_bufferIndex = 0;
	}

	std::string Request::_readLine(bool evenWithoutLF)
	{
		std::string	line;
		size_t		i;
		bool		hasCR = false;

		i = _buffer.find_first_of('\n', _bufferIndex);
		if (i == std::string::npos) {
			if (!evenWithoutLF)
				return ("");
			line = _buffer.substr(_bufferIndex);
			_clearBuffer();
		} else {
			if (i > 0 && _buffer[i - 1] == '\r')
				hasCR = true;
			line = _buffer.substr(_bufferIndex, i - hasCR - _bufferIndex);
			_bufferIndex = i + 1;
			if (_bufferIndex >= _buffer.size())
				_clearBuffer();
			if (line.empty())
				line = "\n";
		}
		return (line);
	}

	bool	Request::_writeBodyTmpFile(int64_t n)
	{
		LOG_DEBUG("HTTP write client request body (" << n << " bytes)");
		errno = 0;
		_tmpFile.clear();
		_tmpFile.write(_buffer.c_str() + _bufferIndex, n);
		if (_tmpFile.fail()) {
			_logError("Error while writing the request body in tmp file");
			LOG_DEBUG("Error (" << errno << "): " << std::strerror(errno));
			_tmpFile.clear();
			_errorCode = 500;
			return (false);
		}
		return (true);
	}

	bool	Request::_createBodyTmpFile()
	{
		_tmpFilename = createRandomFilename();
		LOG_DEBUG("Request body tmp filename: \"" << _tmpFilename << "\"");
		_tmpFile.open(_tmpFilename.c_str(), std::ios::out | std::ios::binary);
		if (_tmpFile.fail()) {
			_logError("Cannot open() the request body tmp file");
			_errorCode = 500;
			return (false);
		}
		return (true);
	}

	void	Request::_closeTmpFile()
	{
		_tmpFile.clear();
		if (_tmpFile.is_open())
			_tmpFile.close();
		if (_tmpFile.fail()) {
			LOG_ERROR("Bad close() on \"" << _tmpFilename << "\"");
			_tmpFile.clear();
		}
	}

	void	Request::_deleteTmpFile()
	{
		if (_tmpFile.is_open())
			_closeTmpFile();
		if (unlink(_tmpFilename.c_str()) < 0) {
			_logError((std::string("unlink(") + _tmpFilename
					   + ") failed: " + std::strerror(errno)).c_str());
		} else
			_tmpFilename.clear();
	}

	void	Request::_clearStartAndFieldLines()
	{
		_requestLine.clear();
		_requestMethod.clear();
		_uri.clear();
		_query.clear();
		_extension.clear();
		_httpVersion.clear();
		_headers.clear();
		_host.clear();
		_contentLength = -1;
		_bodySize = -1;
		_isKeepAlive = true;
		_languages.clear();
		_isChunkedRequest = false;
		_hasReceivedHeaders = false;
	}

	void	Request::clearRequest()
	{
		_serverConfig = 0;
		_location = 0;
		if (!isKeepAlive()) {
			_buffer.clear();
			_bufferIndex = 0;
		} else
			_clearBuffer();
		_clearStartAndFieldLines();
		_errorCode = 0;
		if (_tmpFile.is_open())
			_closeTmpFile();
		if (!_tmpFilename.empty()) {
			_deleteTmpFile();
			_tmpFilename.clear();
		}
		_isTerminatedRequest = false;
		_isInternalRedirect = false;
	}

}	// namespace webserv
