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

	Request::Request(): _serverConfig(0),
						_location(0),
						_requestMethod(EMPTY),
						_isKeepAlive(true),
						_bodySize(-1),
						_isChunkedRequest(false),
						_isTerminatedRequest(false)
	{
		LOG_INFO("New Request instance");
	}

	Request::Request(const Request& src): _serverConfig(src._serverConfig),
								_location(src._location),
								_requestMethod(src._requestMethod),
								_isKeepAlive(src._isKeepAlive),
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

	int	Request::_parseChunkedRequest(const char* buffer,
										const server_configs& serverConfigs)
	{
		// TO DO: parse [check and set Headers & Flags, maxBodySize, etc]
		// 		  if (error) {
		// 		    log error;
		// 		    clearRequest();
		// 		    return (errorCode 4xx or 5xx);
		// 		  }
		// 		  try {
		// 		    save chunk;
		// 		  } catch (const std::exception& e) {
		// 		    log error;
		// 		    clearRequest();
		// 		    return (errorCode 4xx or 5xx);
		// 		  }
		// 		  if (last chunk) {
		// 		    if (!_serverConfig)
		// 		      _loadServerConfig(serverConfigs);
		// 		    set _isTerminatedRequest;
		// 		  }
		// 		  return (0);

		(void)buffer;
		if (!_serverConfig || !_location)
			if (!_loadServerConfig(serverConfigs))
				return (500);
		return (0);
	}

	int	Request::parseRequest(const char* buffer,
								const server_configs& serverConfigs)
	{
		// TO DO: if (_isChunkedRequest)
		// 		    return (_parseChunkedRequest, serverConfigs);
		// 		  parse [check and set Headers & Flags, maxBodySize, etc,
		// 		  		 or set _isChunkedRequest and return (_parseChunked)]
		// 		  if (error) {
		// 		    log error;
		// 		    clearRequest();
		// 		    return (errorCode 4xx or 5xx);
		// 		  }
		//		  if (!_serverConfig)
		//		    _loadServerConfig(serverConfigs);
		// 		  set _isTerminatedRequest;
		// 		  return (0);

		(void)buffer;
		if (!_serverConfig || !_location)
			if (!_loadServerConfig(serverConfigs))
				return (500);
		// check content length header after request header sent
		return (0);
	}

	void	Request::clearRequest()
	{
		// TO DO: clear saved chunks;	// with try-catch if necessary

		_serverConfig = 0;
		_location = 0;
		_requestMethod = EMPTY;
		_host.clear();
		_isKeepAlive = true;
		_isChunkedRequest = false;
		_isTerminatedRequest = false;
	}

}	// namespace webserv
