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
		LOG_INFO("New Request instance");
	}

	Request::Request(const Request& src): _clientSocket(src._clientSocket),
											_serverConfig(0),
											_location(0),
											_isKeepAlive(true),
											_hasReceivedHeaders(false),
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
		// TO DO: clear _chunks & everything necessary, cf. clearRequest()
	}

	/**************************************************************************/
	/*                            MEMBER FUNCTIONS                            */
	/**************************************************************************/

	const std::string&	Request::getServerName() const
	{
		static const std::string	emptyString("");

		if (_serverConfig && _serverConfig->getServerNames().begin()
				!= _serverConfig->getServerNames().end())
			return (*_serverConfig->getServerNames().begin());
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

		(void)recvBuffer;
		(void)unprocessedBuffer;
		if (_hasReceivedHeaders && (!_serverConfig || !_location)) {
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
		// 		  		 and only read the Content-Length amount to prevent
		// 		  		 incomplete messages or reading the next request?;
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
			// And check them when _isTerminatedRequest?
			return (_checkHeaders());
		}
		return (0);
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

	void	Request::clearRequest()
	{
		// TO DO: clear potentially saved chunks, with try-catch if necessary

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
