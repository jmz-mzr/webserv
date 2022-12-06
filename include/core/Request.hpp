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

		const Method::Type&		getRequestMethod() const
										{ return (_requestMethod); }
		const Location*			getLocation() const
										{ return (_location); }

		bool	isChunkedRequest() const { return (_isChunkedRequest); }
		bool	isTerminatedRequest() const { return (_isTerminatedRequest); }
		bool	isKeepAlive() const { return (_isKeepAlive); }
		bool	isInternalRedirect() const { return (_isInternalRedirect); }

		int		parseRequest(std::string& unprocessedBuffer, const char* buffer,
								const server_configs& serverConfigs);

		void	setRequestMethod(const Method::Type& method);
		int		loadInternalRedirect(const std::string& redirectTo);

		void	clearRequest();
	private:
		typedef ServerConfig::locations_map			locations_map;

		Request();

		Request&	operator=(const Request& rhs);

		void			_initHeaders();
		bool			_findHeader(std::string key);
		void    		_setHeader(std::string key, std::string value);
		void			_parse(std::string str);
		void        	_parseMethod(std::string line);
		void        	_parsePath(std::string line);
		std::string		_readLine();
		std::string		_getKey(std::string line);
		std::string		_getValue(std::string line);
		bool			_parseRequestTarget(const std::string& requestTarget);
		void			_parseInternalTarget(const std::string& redirectTo);
		int				_parseChunkedRequest(std::string& unprocessedBuffer,
										const char* buffer,
										const server_configs& serverConfigs);
		bool			_loadServerConfig(const server_configs& serverConfigs);
		bool			_loadLocation(const ServerConfig& serverConfig);
		bool			_loadExtensionLocation(const ServerConfig& serverConfig);
		bool			_loadExtensionLocation(const Location& location);
		int				_checkHeaders();
		int				_checkHost();
		int				_checkMaxBodySize();
		int				_checkMethod();

		std::map<std::string, std::string>	_headers;
		std::string							_body;
		std::string							_buffer;
		std::string							_method;
		std::string::size_type				_bufferIndex;
		const AcceptSocket&					_clientSocket;
		const ServerConfig*					_serverConfig;
		const Location*						_location;

		// TO DO: 1) If the request line is invalid, immediately return 400
		// 2) If the request line is > 8192, immediately return 414
		std::string			_requestLine;

		// TO DO: 1) The uppercase letters, '_' and '-' are valid characters
		// that must be accepted when parsing the method in the request line,
		// that will lead to unallowed methods (like "_GET" or "P-OST")
		// 2) If invalid return 400, if not allowed _checkHeaders will return 405
		Method::Type		_requestMethod;

		// TO DO: 1) It is what comes before '#', or the first '?' starting the args,
		// and after the potential valid full scheme, domain name and port
		// 2) If the requested origin-form URI has no '/' prefix, or if it goes up
		// in directories (with "/..") such that it would go above "/"
		// (like '/../abc'), it should return a 400 error (but '/abc/../cde',
		// '/././.', '/...' are all fine)
		// 3) "/." becomes '/', "//" -> '/', "%4d" -> 'M', "%4E" -> 'N', etc...
		// All the many other rules are in the RFCs (9112 et 3986) and in the
		// NGINX implementation (https://bit.ly/3XEvVs1)
		std::string			_uri;

		// TO DO: It is what comes after the first '?' in the URI
		std::string			_args;

		// TO DO: It is what comes after the last '.' in the URI part, but only
		// if it is not part of "/.." that goes up in a directory, if it is
		// preceded and follwed by a valid usual character
		// It is used to set the Response's "Content-Type"
		std::string			_ext;

		std::string			_host;

		// TO DO: 1) Ignore 'Keep-Alive' Header and do not implement "timeout"
		// and "max" parameters: "HTTP/1.1 does not define any parameters"
		// 2) Only set it to false if 'Connection' Header is set to "close"
		bool				_isKeepAlive;
		bool				_hasReceivedHeaders;

		// TO DO: The Content-Length also limits the size of what is actually
		// going to be processed from the body (even if it is longer)
		long long			_bodySize;

		bool				_isChunkedRequest;

		// TO DO: For upload with CGI
//		std::ofstream		_chunks;	// or with a swap space?

		bool				_isTerminatedRequest;
		bool				_isInternalRedirect;
	};

}	// namespace webserv

#endif	// REQUEST_HPP
