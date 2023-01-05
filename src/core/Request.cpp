#include <algorithm>

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
												_isKeepAlive(true),
												_hasReceivedHeaders(false),
												_bodySize(-1),
												_isChunkedRequest(false),
												_isTerminatedRequest(false),
												_isInternalRedirect(false)
	{
		_initHeaders();
		LOG_INFO("New Request instance");
	}

	Request::Request(const Request& src): _clientSocket(src._clientSocket),
								_serverConfig(src._serverConfig),
								_location(src._location),
								_requestLine(src._requestLine),
								_requestMethod(src._requestMethod),
								_uri(src._uri),
								_host(src._host),
								_isKeepAlive(src._isKeepAlive),
								_hasReceivedHeaders(src._hasReceivedHeaders),
								_bodySize(src._bodySize),
								_isChunkedRequest(src._isChunkedRequest),
								_isTerminatedRequest(src._isTerminatedRequest),
								_isInternalRedirect(src._isInternalRedirect)
	{
		// TO DO: handle swap of std::ofstream (or other _chunks object)
		// 		  or not if Request in only copied at Client creation?
		_initHeaders();
		LOG_INFO("Request copied");
	}

	/**************************************************************************/
	/*                            MEMBER FUNCTIONS                            */
	/**************************************************************************/

	std::string	Request::_getServerName() const
	{
		if (_serverConfig->getServerNames().begin()
				== _serverConfig->getServerNames().end())
			return (*_serverConfig->getServerNames().begin());
		return ("");
	}

	int	Request::_checkHost() const
	{
		if (_host.empty()) {
			LOG_INFO("Client sent HTTP/1.1 request without \"Host\" header"
					<< " while reading client request headers, client: "
					<< _clientSocket.getIpAddr() << ":"
					<< _clientSocket.getPort() << ", server: "
					<< ft_inet_ntoa(_serverConfig->getListenPair().sin_addr)
					<< ":" << ntohs(_serverConfig->getListenPair().sin_port)
					<< " (\"" << _getServerName() << "\"), request: \""
					<< _requestLine << "\"");
			return (400);
		}
		return (0);
	}

	int Request::_checkMaxBodySize() const
	{
		LOG_DEBUG("Content-Length: " << _bodySize << ", max: "
				<< _location->getMaxBodySize());
		if (_bodySize > 0 && _bodySize > _location->getMaxBodySize()) {
			LOG_ERROR("Client intended to send too large body: " << _bodySize
					<< " bytes, client: " << _clientSocket.getIpAddr() << ":"
					<< _clientSocket.getPort() << ", server: "
					<< ft_inet_ntoa(_serverConfig->getListenPair().sin_addr)
					<< ":" << ntohs(_serverConfig->getListenPair().sin_port)
					<< " (\"" << _getServerName() << "\"), request: \""
					<< _requestLine << "\", host: \"" << _host << "\"");
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
			LOG_ERROR("Access forbidden by rule, client: "
					<< _clientSocket.getIpAddr() << ":"
					<< _clientSocket.getPort() << ", server: "
					<< ft_inet_ntoa(_serverConfig->getListenPair().sin_addr)
					<< ":" << ntohs(_serverConfig->getListenPair().sin_port)
					<< " (\"" << _getServerName() << "\"), request: \""
					<< _requestLine << "\", host: \"" << _host << "\"");
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
		locations_map::const_iterator		extLocation;

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
		locations_map::const_iterator		extLocation;

		extLocation = location.getLocations().lower_bound("*.~");
		while (extLocation != location.getLocations().end()
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
		locations_map::const_iterator		location;

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
				if (ft_strcmp_icase(_host, *name) == 0) {
					_serverConfig = &(*config);
					LOG_DEBUG("Using server: \"" << *name << "\" (on \""
							<< ft_inet_ntoa(_serverConfig->getListenPair().sin_addr)
							<< ":"
							<< ntohs(_serverConfig->getListenPair().sin_port)
							<< "\")");
					return (true);
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
		// TO DO: parse: [=> use unprocessedBuff first, then recvBuff]
		// 		  		 if (!_hasReceivedHeaders)
		// 		  		   check and set Headers, returning 400 error if an
		// 		  		   existing header has an incorrect value, or if a
		// 		  		   request header field is too long (> 8192);
		// 		  		   and only read the Content-Length amount to prevent
		// 		  		   incomplete messages or reading the next request?;
		//				 save what is not read in unprocessedBuff;
		// 		  		 set _isChunkedRequest/_isTerminatedRequest if needed;
		// 		  if (error) {
		// 		    log error;
		// 		    clearRequest(); // only if closing connection error?
		// 		    return (errorCode 4xx or 5xx);
		// 		  }
		// 		  try {	// or try-catch in client?
		// 		    save chunk;
		// 		  } catch (const std::exception& e) {
		// 		    log error;
		// 		    clearRequest(); // only if closing connection error?
		// 		    return (errorCode 4xx or 5xx);
		// 		  }
		// 		  if (last chunk) {
		// 		    set _isTerminatedRequest;
		//		  if (!_hasReceivedHeaders && not loaded config) {
		//		    _loadServerConfig(serverConfigs);
		//		    return (_checkHeaders);
		//		  }
		// 		  return (0);


		// headers are already filled by normal parsing
		// we only need to parse the body and add/remove the footers
		// TODO : chunked encoding

		(void)recvBuffer;
		(void)unprocessedBuffer;
		if (_hasReceivedHeaders && (!_serverConfig || !_location)) {
			if (!_loadServerConfig(serverConfigs))
				return (500);
			// After header sent, check content length header, etc
			// Or/And check them when _isTerminatedRequest?
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
			LOG_INFO("Client sent invalid request while reading client request"
					<< " line, client: " << _clientSocket.getIpAddr() << ":"
					<< _clientSocket.getPort() << ", server: "
					<< ft_inet_ntoa(_serverConfig->getListenPair().sin_addr)
					<< ":" << ntohs(_serverConfig->getListenPair().sin_port)
					<< " (\"" << _getServerName() << "\"), request: \""
					<< _requestLine << "\"");
			return (false);
		}
		return (true);
	}

	int	Request::parseRequest(std::string& unprocessedBuffer,
								const char* recvBuffer,
								const server_configs& serverConfigs)
	{
		_buffer = (unprocessedBuffer + recvBuffer);
		//the "\r\n\r\n" pattern marks the end of the header section
		size_t i = _buffer.find("\r\n\r\n");
		if (i != std::string::npos)
		{
			//the content-length header is MANDATORY for messages with
			// entity bodies (RFC)
			//That means that if content length isn't specified, we should start
			// parsing when all the headers are received
			if (_buffer.find("Content-Length: ") == std::string::npos)
			{
				// add the excess into unprocessedBuffer
				if (i + 4 < _buffer.size())
				{
					unprocessedBuffer = _buffer.substr(i + 4, std::string::npos);
					_buffer.erase(i + 4, std::string::npos);
				}
				_parse(_buffer);
				// If the encoding is chunked, we parse the body
				if (_headers["Transfer-Encoding"] == "chunked")
					_parseChunkedRequest(unprocessedBuffer, recvBuffer, serverConfigs);
			}
			else
			{
				size_t len = static_cast<size_t>(
					std::atoi(_headers["Content-Length"].c_str() + i + 4));
				//	if Content-Length header is present, we wait to receive
				//	body datas before processing
				// We know we received the entire body when buffer length
				// is equal to or greater than Content-Length
				if (len > 0 && _buffer.size() >= len)
				{
					_parse(_buffer);
					_body = _buffer.substr(_bufferIndex, len);
					// if body is greater, we add the excess into unprocessedbuffer
					// for the next message
					//TODO[ISSUES] : The logic is wrong
					if (_buffer.size() > len)
						unprocessedBuffer = _buffer.substr(len, std::string::npos);
				}
			}
		}
		else
			unprocessedBuffer = _buffer;
		_buffer.clear();
		if (_hasReceivedHeaders && (!_serverConfig || !_location)) {
			if (!_loadServerConfig(serverConfigs))
				return (500);
			// After header sent, check content length header, etc
			// Or/And check them when _isTerminatedRequest?
			return (_checkHeaders());
		}
		return (0);
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
					double weight = static_cast<double>(
						atof(values[1].c_str() + 2));
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
		size_t	args = redirectTo.find('?');

		_uri = redirectTo.substr(0, args);
		if (args != std::string::npos)
			_args = redirectTo.substr(args + 1);
		else
			_args.clear();
	}

	int	Request::loadInternalRedirect(const std::string& redirectTo)
	{
		// TO DO: During internal redirections, NGINX does not check the URI
		// validity (!!!) and allows it to go up like "/../../../[file]"

		_isInternalRedirect = true;
		if (redirectTo.empty() || redirectTo[0] != '/') {
			LOG_DEBUG("Incorrect internal redirect: \"" << redirectTo << "\"");
			return (500);
		}
		_parseInternalTarget(redirectTo);
		LOG_DEBUG("Internal redirect: \"" << _uri << "?" << _args << "\"");
		if (!_loadLocation(*_serverConfig))
			return (500);
		return (_checkHeaders());
	}

	void	Request::clearRequest()
	{
		// TO DO: clear saved chunks;	// with try-catch if necessary

		_serverConfig = 0;
		_location = 0;
		_requestMethod.clear();
		_host.clear();
		_isKeepAlive = true;
		_hasReceivedHeaders = false;
		_bodySize = -1;
		_isChunkedRequest = false;
		_isTerminatedRequest = false;
		_isInternalRedirect = false;
	}

}	// namespace webserv
