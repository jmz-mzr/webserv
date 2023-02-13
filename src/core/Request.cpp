#include <algorithm>
#include <cerrno>
#include <cstring>

#include <unistd.h>

#include "core/Request.hpp"
#include "webserv_config.hpp"
#include "utils/Logger.hpp"
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
												_code(0),
												_isKeepAlive(true),
												_hasReceivedHeaders(false),
												_hasReceivedBody(false),
												_hasBody(false),
												_bodySize(-1),
												_isChunkedRequest(false),
												_isTerminatedRequest(false),
												_isInternalRedirect(false)
	{
		_initHeaders();
		LOG_INFO("New Request instance");
	}

	Request::Request(const Request& src): _clientSocket(src._clientSocket),
											_serverConfig(0),
											_location(0),
											_code(0),
											_isKeepAlive(true),
											_hasReceivedHeaders(false),
											_hasReceivedBody(false),
											_hasBody(false),
											_bodySize(-1),
											_isChunkedRequest(false),
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

	void	Request::_logError(const char* errorAt) const
	{
		LOG_ERROR(errorAt << ", client: " << _clientSocket.getIpAddr()
				<< ":" << _clientSocket.getPort() << ", server: "
				<< ft_inet_ntoa(_serverConfig->getListenPair().sin_addr)
				<< ":" << ntohs(_serverConfig->getListenPair().sin_port)
				<< " (\"" << getServerName() << "\"), request: \""
				<< _requestLine << "\", host: \"" << _host << "\""
				<< ", body size: " << _bodySize);
	}

	int	Request::_checkHost() const
	{
		if (_host.empty()) {
			_logError("Client sent HTTP/1.1 request without \"Host\" header");
			return (400);
		}
		return (0);
	}

	int Request::_checkMaxBodySize() const
	{
		LOG_DEBUG("Content-Length: " << _bodySize << ", max: "
				<< _location->getMaxBodySize());
		if (_bodySize > 0 && _bodySize > _location->getMaxBodySize()) {
			_logError("Client intended to send too large body");
			return (413);
		}
		return (0);
	}

	int	Request::_checkMethod() const
	{
		if (_requestMethod.empty())
			return (405);
		if (!_location->getLimitExcept().empty()
				&& !_location->getLimitExcept().count(_requestMethod)) {
			_logError("Access forbidden by rule");
			return (403);
		}
		return (0);
	}

	int	Request::_checkHeaders() const
	{
		int		responseCode;

		if ((responseCode = _checkHost()) != 0)
			return (responseCode);
		if ((responseCode = _checkMaxBodySize()) != 0)
			return (responseCode);
		if ((responseCode = _checkMethod()) != 0)
			return (responseCode);
		return (responseCode);
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

	int	Request::_parseChunkedRequest(std::string& unprocessedBuffer,
										const char* recvBuffer,
										const server_configs& serverConfigs)
	{
		// headers are already filled by normal parsing
		// we only need to parse the body and add/remove the footers
		(void)recvBuffer;
		(void)unprocessedBuffer;

		std::string	processedBody;
		std::string	chunkedBody = _buffer.substr(_bufferIndex);
		long		chunkSize;
		size_t		i = 0;

		chunkSize = strtol(chunkedBody.c_str(), NULL, 16);

		//Documentation : www.jmarshall.com/easy/http
		while (chunkSize)
		{
			i = chunkedBody.find("\r\n", i) + 2;
			processedBody += chunkedBody.substr(i,
					static_cast<unsigned long>(chunkSize));
			i += static_cast<unsigned long>(chunkSize + 2);
			chunkSize = strtol(chunkedBody.c_str() + i, NULL, 16);
		}
		if (chunkedBody.find("\r\n0\r\n") == chunkedBody.size() - 5)
			_hasReceivedBody = true;
		_body += processedBody;

		if (_hasReceivedBody && _hasReceivedHeaders
		&& (!_serverConfig || !_location)) {
			if (!_loadServerConfig(serverConfigs))
				return (500);
			// After header sent, check content length header, etc
			// And check them when _isTerminatedRequest?
			return (_checkHeaders());
		}
		return (0);
	}

	bool	Request::_parseRequestTarget(const std::string& requestTarget)
	{
		// TO DO: This is a temp version of the function, used for the internal
		// redirections, and mainly for parsing the request line (after parsing
		// the Method, and before parsing the HTTP version)
		// All the many rules to correctly parse the request line are in the
		// RFCs (9112 et 3986) and in the NGINX code (https://bit.ly/3XEvVs1)

		bool	error = false;

		_uri = requestTarget;
		if (error) {
			_logError("Client sent invalid request line");
			return (false);
		}
		return (true);
	}

	bool	Request::_isChunkEnd()
	{
		std::string	chunkedBody = _buffer.substr(_bufferIndex);

		if (chunkedBody.find("\r\n0\r\n") == chunkedBody.size() - 5)
			return true;
		return false;
	}

	size_t	Request::_fullRequestReceived()
	{
		size_t	i = 0;

		//"CRLFCRLF" or "\r\n\r\n" pattern marks the end of the header section
		if ((i = _buffer.find("\r\n\r\n")) != std::string::npos
			|| (i = _buffer.find("\r\n\n")) != std::string::npos
			|| (i = _buffer.find("\n\r\n")) != std::string::npos
			|| (i = _buffer.find("\n\n")) != std::string::npos)
			return i;
		return std::string::npos;
	}

	int	Request::parseRequest(std::string& unprocessedBuffer,
								const char* recvBuffer,
								const server_configs& serverConfigs)
	{

		LOG_DEBUG("---PARSING REQUEST---");

		_buffer = (unprocessedBuffer + recvBuffer);
		size_t i = _fullRequestReceived();

		
		if (i != std::string::npos)
		{
			if (_tmpFilename.empty())
			{
				std::stringstream ss;
				struct timeval	time_now;

				ss << gettimeofday(&time_now, NULL);
				_tmpFilename = "body_" + ss.str();
				//TODO : figure out where to close the streams
				_tmpFileStream.open(_tmpFilename.c_str(), std::ofstream::binary);
				if (!_tmpFileStream.is_open())
					return (500);
				_requestFileStream.open(_tmpFilename.c_str(), std::ifstream::binary);
				if (!_requestFileStream.is_open())
					return (500);
			}
			//If content length isn't specified, we should start
			//parsing when all the headers are received (RFC)
			if (_buffer.find("Content-Length: ") == std::string::npos && !_hasBody)
			{
				//we discard excess buffer
				_buffer = _buffer.substr(0, i + 4);
				_parse(_buffer);
				if (_code != 0)
					return (_code);
				// If the encoding is chunked, we parse the body
				if (_headers["Transfer-Encoding"] == "chunked" && _isChunkEnd())
				{
					_hasBody = true;
					_parseChunkedRequest(unprocessedBuffer, recvBuffer,
							serverConfigs);
					_tmpFileStream << _body;
				}
				else
					unprocessedBuffer = _buffer;
			}
			else
			{
				// the request has a body because Content Length header exists
				_hasBody = true;
				std::string body_size = _buffer.substr(
						_buffer.find("Content-Length: ")
						+ strlen("Content-Length: ") , 10);
				// HTTP Request body size
				_bodySize = static_cast<int64_t>(std::atoi(body_size.c_str()));
				//	if Content-Length header is present, we wait to receive
				//	the entire request body before processing
				if (_bodySize > 0 && _buffer.size() >=
						static_cast<unsigned long>(_bodySize) + i + 4)
				{
					_parse(_buffer);
					if (_code != 0)
						return (_code);
					_body = _buffer.substr(_bufferIndex,
							static_cast<unsigned long> (_bodySize));
					_tmpFileStream << _body;
					_hasReceivedBody = true;
				}
				else
					unprocessedBuffer += _buffer;
			}
		}
		// if we don't have received all the headers yet
		// we save the request in unprocessedBuffer
		else
		{
			unprocessedBuffer += _buffer;
			if (_buffer.length() < RECV_BUFFER_SIZE)
			{
				_code = 400;
				return (_code);
			}
		}
		_buffer.clear();
		_isTerminatedRequest = true;
		if (( (_hasBody && _hasReceivedBody && _hasReceivedHeaders)
		|| (!_hasBody && _hasReceivedHeaders) )
		&& (!_serverConfig || !_location)) {
			_isTerminatedRequest = true;
			LOG_DEBUG("status code : " << _code);
			if (!_loadServerConfig(serverConfigs))
			{
				_code = 500;
				return (_code);
			}
			// After header sent, check content length header, etc
			// And check them when _isTerminatedRequest?
			return (_checkHeaders());
		}
		return (_code);
	}

	//set the accepted languages values in a map sorted by quality value
	void	Request::_setLanguage()
	{
		std::string	header = this->_headers["Accept-Language"];
		std::vector<std::string> language_values;

		if (header != "")
		{
			//Trim all spaces and separate the languages, the separator is ","
			header = ft_string_remove(header, ' ');
			language_values = ft_string_split(header, ",");
			//insert the languages with their quality value
			for (std::vector<std::string>::iterator it = language_values.begin();
			it != language_values.end(); it++)
			{
				std::vector<std::string> values;
				values = ft_string_split(*it, ";");
				// when no quality value is specified, the default value is 1.0
				if (values.size() == 1)
					_languages.insert(std::make_pair(1.0, values[0]));
				else
				{
					//the quality value starts with "q=" example : "q=0.8"
					//we need to remove the 2 first characters before conversion
					double weight = atof(values[1].c_str() + 2);
					_languages.insert(std::make_pair(weight, values[0]));
				}
			}
		}
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
		return (_checkHeaders());
	}

	void	Request::_closeTmpFile()
	{
		_tmpFile.clear();
		_tmpFile.close();
		if (_tmpFile.fail()) {
			LOG_ERROR("Bad close() on \"" << _tmpFilename << "\"");
			_tmpFile.clear();
		}
	}

	void	Request::_deleteTmpFile()
	{
		if (unlink(_tmpFilename.c_str()) < 0) {
			_logError((std::string("unlink(") + _tmpFilename
					   + ") failed: " + strerror(errno)).c_str());
		}
	}

	void	Request::clearRequest()
	{
		_serverConfig = 0;
		_location = 0;
		_requestLine.clear();
		_requestMethod.clear();
		_uri.clear();
		_query.clear();
		_extension.clear();
		_contentType.clear();
		_host.clear();
		_isKeepAlive = true;
		_hasReceivedHeaders = false;
		_bodySize = -1;
		_isChunkedRequest = false;
		_tmpString.clear();
		if (_tmpFile.is_open())
			_closeTmpFile();
		if (!_tmpFilename.empty()) {
			_deleteTmpFile();
			_tmpFilename.clear();
		}
		_tmpFilename.clear();
		_isTerminatedRequest = false;
		_isInternalRedirect = false;
	}

}	// namespace webserv
