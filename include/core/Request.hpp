#ifndef REQUEST_HPP
# define REQUEST_HPP

# include <vector>
# include <string>

# include "config/ServerConfig.hpp"
# include "config/Location.hpp"

namespace	webserv
{

	class	ServerConfig;

	class	Request {
	public:
		typedef std::vector<ServerConfig>	server_configs;

		Request();
		Request(const Request& src);
		~Request() { }	// clear _chunks if not automatic

		const int&		getRequestMethod() const { return (_requestMethod); }

		bool	isChunkedRequest() const { return (_isChunkedRequest); }
		bool	isTerminatedRequest() const { return (_isTerminatedRequest); }
		bool	isKeepAlive() const { return (_isKeepAlive); }

		int		parseRequest(std::string& unprocessedBuffer, const char* buffer,
								const server_configs& serverConfigs);

		void	clearRequest();
	private:
		typedef ServerConfig::locations_map			locations_map;

		Request&	operator=(const Request& rhs);

		int		_parseChunkedRequest(std::string& unprocessedBuffer,
										const char* buffer,
										const server_configs& serverConfigs);
		bool	_loadServerConfig(const server_configs& serverConfigs);
		bool	_loadLocation(const ServerConfig& serverConfig);
		bool	_loadExtensionLocation(const ServerConfig& serverConfig);
		bool	_loadExtensionLocation(const Location& location);

		const ServerConfig*	_serverConfig;
		const Location*		_location;
		int					_requestMethod;

		// TO DO: If the requested uri has no "/" prefix, or if it goes up in
		// directories (with '..') such that it would go above "/"
		// (like '/../abc'), it should return a 400 error (but '/abc/../cde',
		// '/././.', '/...' are all fine)
		std::string			_requestLine;

		std::string			_uri;

		// TO DO: 1) Record in lowercase with ft_str_tolower
		// 2) If none, or empty, return 400 error
		std::string			_host;

		bool				_isKeepAlive;	// divide in a Class with parameters?
		bool				_hasReceivedHeaders;
		long long			_bodySize;
		bool				_isChunkedRequest;
//		std::ofstream		_chunks;	// or with a swap space?
		bool				_isTerminatedRequest;
	};

}	// namespace webserv

#endif	// REQUEST_HPP
