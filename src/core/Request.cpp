#include "core/Request.hpp"
#include "utils/Logger.hpp"
#include "utils/utils.hpp"

namespace	webserv
{

	/**************************************************************************/
	/*                       CONSTRUCTORS / DESTRUCTORS                       */
	/**************************************************************************/

	Request::Request(): _requestMethod(kEmpty),
						_isKeepAlive(true),
						_isChunkedRequest(false),
						_isTerminatedRequest(false)
	{
		LOG_INFO("New Request instance");
	}

	Request::Request(const Request& src): _requestMethod(src._requestMethod),
								_isKeepAlive(src._isKeepAlive),
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

	const enum RequestMethod&	Request::getRequestMethod() const
	{
		return (_requestMethod);
	}

	void	Request::_loadServerConfig(const server_configs& serverConfigs)
	{
		server_configs::const_iterator				config;
		std::vector<std::string>::const_iterator	name;

		config = serverConfigs.begin();
		while (config != serverConfigs.end()) {
			name = config->getNames().begin();
			while (name != config->getNames().end()) {
				if (ft_strcmp_icase(_hostName, *name) == 0) {
					_serverConfig = *config;
					return ;
				}
				++name;
			}
			++config;
		}
		_serverConfig = serverConfigs[0];
	}

	int	Request::_parseChunkedRequest(const char* buffer,
										const server_configs& serverConfigs)
	{
		// TO DO: parse [check and set Headers & Flags, serverConfig, etc]
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
		// 		  if (last chunk)
		// 		    set _isTerminatedRequest;
		// 		  return (0);

		(void)buffer;
		if (_serverConfig.empty())
			_loadServerConfig(serverConfigs);
		return (0);
	}

	int	Request::parseRequest(const char* buffer,
								const server_configs& serverConfigs)
	{
		// TO DO: if (_isChunkedRequest)
		// 		    return (_parseChunkedRequest, serverConfigs);
		// 		  parse [check and set Headers & Flags, serverConfig, etc,
		// 		  		 or set _isChunkedRequest and return (_parseChunked)]
		// 		  if (error) {
		// 		    log error;
		// 		    clearRequest();
		// 		    return (errorCode 4xx or 5xx);
		// 		  }
		// 		  set _isTerminatedRequest;
		// 		  return (0);

		(void)buffer;
		if (_serverConfig.empty())
			_loadServerConfig(serverConfigs);
		return (0);
	}

	void	Request::clearRequest()
	{
		// TO DO: clear saved chunks;	// with try-catch if necessary

		_serverConfig.clearConfig();
		_requestMethod = kEmpty;
		_hostName.clear();
		_isKeepAlive = true;
		_isChunkedRequest = false;
		_isTerminatedRequest = false;
	}

}	// namespace webserv
