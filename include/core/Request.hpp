#ifndef REQUEST_HPP
# define REQUEST_HPP

# include <vector>
# include <string>

# include "config/ServerConfig.hpp"
# include "core/AcceptSocket.hpp"
# include "config/Location.hpp"

namespace	webserv
{

	class	Request {
	public:
		typedef std::vector<ServerConfig>	server_configs;

		Request(const AcceptSocket& clientSocket);
		Request(const Request& src);
		~Request() { }	// clear _chunks if not automatic

		const Method::Type&		getRequestMethod() const { return (_requestMethod); }

		bool	isChunkedRequest() const { return (_isChunkedRequest); }
		bool	isTerminatedRequest() const { return (_isTerminatedRequest); }
		bool	isKeepAlive() const { return (_isKeepAlive); }

		int		parseRequest(std::string& unprocessedBuffer, const char* buffer,
								const server_configs& serverConfigs);

		void	clearRequest();
	private:
		typedef ServerConfig::locations_map			locations_map;

		Request();

		Request&	operator=(const Request& rhs);

		int		_parseChunkedRequest(std::string& unprocessedBuffer,
										const char* buffer,
										const server_configs& serverConfigs);
		bool	_loadServerConfig(const server_configs& serverConfigs);
		bool	_loadLocation(const ServerConfig& serverConfig);
		bool	_loadExtensionLocation(const ServerConfig& serverConfig);
		bool	_loadExtensionLocation(const Location& location);
		int		_checkHeaders();
		int		_checkHost();
		int		_checkMaxBodySize();
		int		_checkMethod();

		const AcceptSocket&	_clientSocket;
		const ServerConfig*	_serverConfig;
		const Location*		_location;
		Method::Type		_requestMethod;

		// TO DO: If the request line is incorrect, immediately return 400
		std::string			_requestLine;

		// TO DO: If the requested uri has no "/" prefix, or if it goes up in
		// directories (with '..') such that it would go above "/"
		// (like '/../abc'), it should return a 400 error (but '/abc/../cde',
		// '/././.', '/...' are all fine)
		std::string			_uri;

		std::string			_host;
		bool				_isKeepAlive;	// divide in a Class with parameters?
		bool				_hasReceivedHeaders;

		// TO DO: The Content-Length also limits the size of what is actually
		// going to be processed from the body (even if it is longer)
		long long			_bodySize;

		bool				_isChunkedRequest;

		// TO DO: For upload with CGI
//		std::ofstream		_chunks;	// or with a swap space?

		bool				_isTerminatedRequest;
	};

}	// namespace webserv

#endif	// REQUEST_HPP
