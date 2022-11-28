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
												_requestMethod(Method::kEmpty),
												_isKeepAlive(true),
												_hasReceivedHeaders(false),
												_bodySize(-1),
												_isChunkedRequest(false),
												_isTerminatedRequest(false)
	{
		LOG_INFO("New Request instance");
	}

	Request::Request(const Request& src): _clientSocket(src._clientSocket),
								_serverConfig(src._serverConfig),
								_location(src._location),
								_requestMethod(src._requestMethod),
								_requestLine(src._requestLine),
								_uri(src._uri),
								_host(src._host),
								_isKeepAlive(src._isKeepAlive),
								_hasReceivedHeaders(src._hasReceivedHeaders),
								_bodySize(src._bodySize),
								_isChunkedRequest(src._isChunkedRequest),
								_isTerminatedRequest(src._isTerminatedRequest)
	{
		// TO DO: handle swap of std::ofstream (or other _chunks object)
		// 		  or not if Request in only copied at Client creation?

		LOG_INFO("Request copied");
	}

	/**************************************************************************/
	/*                            MEMBER FUNCTIONS                            */
	/**************************************************************************/

	int	Request::_checkHost()
	{
		if (_host.empty()) {
			LOG_INFO("Client sent HTTP/1.1 request without \"Host\" header"
					<< " while reading client request headers, client: "
					<< _clientSocket.getIpAddr() << ":"
					<< _clientSocket.getPort() << ", server: "
					<< _serverConfig->getListenPairs()[0].first << ":"
					<< _serverConfig->getListenPairs()[0].second << " (\""
					<< _serverConfig->getServerNames()[0] << "\"), request: \""
					<< _requestLine << "\"");
			return (400);
		}
		return (0);
	}

	int Request::_checkMaxBodySize()
	{
		LOG_DEBUG("Content-Length: " << _bodySize << ", max: "
				<< _location->getMaxBodySize());
		if (_bodySize > 0 && _bodySize > _location->getMaxBodySize()) {
			LOG_ERROR("Client intended to send too large body: " << _bodySize
					<< " bytes, client: " << _clientSocket.getIpAddr() << ":"
					<< _clientSocket.getPort() << ", server: "
					<< _serverConfig->getListenPairs()[0].first << ":"
					<< _serverConfig->getListenPairs()[0].second << " (\""
					<< _serverConfig->getServerNames()[0] << "\"), request: \""
					<< _requestLine << "\", host: \"" << _host << "\"");
			return (413);
		}
		return (0);
	}

	int	Request::_checkMethod()
	{
		if (!(_location->getLimitExcept() & _requestMethod)) {
			LOG_ERROR("Access forbidden by rule, client: "
					<< _clientSocket.getIpAddr() << ":"
					<< _clientSocket.getPort() << ", server: "
					<< _serverConfig->getListenPairs()[0].first << ":"
					<< _serverConfig->getListenPairs()[0].second << " (\""
					<< _serverConfig->getServerNames()[0] << "\"), request: \""
					<< _requestLine << "\", host: \"" << _host << "\"");
			return (403);
		}
		return (0);
	}

	int	Request::_checkHeaders()
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
		clearRequest();
		return (false);
	}

	bool	Request::_loadServerConfig(const server_configs& serverConfigs)
	{
		server_configs::const_iterator				config;
		std::vector<std::string>::const_iterator	name;

		config = serverConfigs.begin();
		while (config != serverConfigs.end()) {
			name = config->getServerNames().begin();
			while (name != config->getServerNames().end()) {
				if (ft_strcmp_icase(_host, *name) == 0) {
					_serverConfig = &(*config);
					LOG_DEBUG("Using server: \"" << *name << "\" (on \""
							<< _serverConfig->getListenPairs()[0].first << ":"
							<< _serverConfig->getListenPairs()[0].second
							<< "\")");
					return (true);
				}
				++name;
			}
			++config;
		}
		_serverConfig = &(serverConfigs[0]);
		LOG_DEBUG("Using default server (on \""
				<< _serverConfig->getListenPairs()[0].first << ":"
				<< _serverConfig->getListenPairs()[0].second << "\")");
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

	int	Request::parseRequest(std::string& unprocessedBuffer,
								const char* recvBuffer,
								const server_configs& serverConfigs)
	{
		// TO DO: if (_isChunkedRequest)
		// 		    return (_parseChunkedRequest(...));
		// 		  parse: [=> use unprocessedBuff first, then recvBuff]
		// 		  		 check and set Headers, returning 400 error if an
		// 		  		 existing header has an incorrect value;
		//				 save what is not read in unprocessedBuff;
		// 		  		 set _isChunked/_hasReceived/_isTerminated if needed;
		// 		  if (error) {
		// 		    log error;
		// 		    clearRequest(); // only if closing connection error?
		// 		    return (errorCode 4xx or 5xx);
		// 		  }
		//		  if (!_hasReceivedHeaders && not loaded config) {
		//		    _loadServerConfig(serverConfigs);
		//		    return (_checkHeaders);
		//		  }
		// 		  return (0);

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

	void	Request::clearRequest()
	{
		// TO DO: clear saved chunks;	// with try-catch if necessary

		_serverConfig = 0;
		_location = 0;
		_requestMethod = Method::kEmpty;
		_host.clear();
		_isKeepAlive = true;
		_hasReceivedHeaders = false;
		_bodySize = -1;
		_isChunkedRequest = false;
		_isTerminatedRequest = false;
	}

}	// namespace webserv
