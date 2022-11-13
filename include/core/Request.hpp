#ifndef REQUEST_HPP
# define REQUEST_HPP

# include <vector>
# include <string>

# include "config/ServerConfig.hpp"

namespace	webserv
{

	enum RequestMethod {
		kEmpty,
		kGet,
		kPost,
		kDelete
	};

	class	Request {
	public:
		typedef std::vector<ServerConfig>	server_configs;

		Request();
		Request(const Request& src);
		~Request() { }	// clear _chunks if not automatic

		const enum RequestMethod&	getRequestMethod() const;

		bool	isChunkedRequest() const { return (_isChunkedRequest); }
		bool	isTerminatedRequest() const { return (_isTerminatedRequest); }
//		bool	isKeepAlive() const;

		int		parseRequest(const char* buffer,
								const server_configs& serverConfigs);

		void	clearRequest();
	private:
		Request&	operator=(const Request& rhs);

		int		_parseChunkedRequest(const char* buffer,
										const server_configs& serverConfigs);
		void	_loadServerConfig(const server_configs& serverConfigs);

		ServerConfig		_serverConfig;
		enum RequestMethod	_requestMethod;
		std::string			_hostName;	// recorded in lowercase with ft_str_tolower
										// if none or empty return 400 error
		bool				_isChunkedRequest;
//		std::ofstream		_chunks;	// or with a swap space?
		bool				_isTerminatedRequest;
//		bool				_keepAlive;	// divide in a Class with parameters
//										// or use with SO_KEEPALIVE
	};

}	// namespace webserv

#endif	// REQUEST_HPP
