#ifndef REQUEST_HPP
# define REQUEST_HPP

# include <vector>
# include <string>
# include <map>
# include <fstream>
# include <sys/time.h>
# include <sstream>
# include <iostream>

# include "config/ServerConfig.hpp"
# include "core/AcceptSocket.hpp"
# include "config/Location.hpp"
# include "utils/utils.hpp"

# define URI_MAX_LENGTH	2048

namespace	webserv
{
	class	Request {
	public:
		typedef std::vector<ServerConfig>				server_configs;
		typedef std::map<std::string, std::string,
							strcmp_icase>				header_map;

		Request(const AcceptSocket& clientSocket);
		Request(const Request& src);
		~Request();

		const AcceptSocket&		getClientSocket () const
										{ return (_clientSocket); }
		const std::string&		getRequestLine() const
										{ return (_requestLine); }
		const std::string&		getRequestMethod() const
										{ return (_requestMethod); }
		const std::string&		getUri() const
										{ return (_uri); }
		const std::string&		getQuery() const
										{ return (_query); }
		const std::string&		getExtension() const
										{ return (_extension); }
		const std::string&		getContentType() const
										{ return (_contentType); }
		const std::string&		getHost() const
										{ return (_host); }
		int64_t					getBodySize() const
										{ return (_bodySize); }
		const header_map&		getHeaders() const
										{ return (_headers); }
		std::fstream&			getTmpFile()
										{ return (_tmpFile); }
		const std::string&		getTmpFilename() const
										{ return (_tmpFilename); }

		const std::string&		getServerName() const;
		const ServerConfig*		getServerConfig() const
										{ return (_serverConfig); }
		const Location*			getLocation() const
										{ return (_location); }

		bool	isChunkedRequest() const { return (_isChunkedRequest); }
		bool	isTerminatedRequest() const { return (_isTerminatedRequest); }
		bool	isKeepAlive() const { return (_isKeepAlive); }
		bool	isInternalRedirect() const { return (_isInternalRedirect); }

		int		parseRequest(std::string& unprocessedBuffer, const char* buffer,
								const server_configs& serverConfigs);

		void	setRequestMethod(const std::string& method);
		int		loadInternalRedirect(const std::string& redirectTo);

		void	clearRequest();
	private:
		typedef ServerConfig::location_map				_location_map;

		Request();

		Request&	operator=(const Request& rhs);

		void			_initHeaders();
		bool			_findHeader(const std::string& key);
		void    		_setHeader(const std::string& key, const std::string& value);
		void			_parse(const std::string& str);
		void        	_parseMethod(const std::string& line);
		void        	_parsePath(const std::string& line);
		void			_checkVersion(const std::string& line);
		std::string		_readLine();
		std::string		_getKey(const std::string& line);
		std::string		_getValue(const std::string& line);
//		std::string		_formatValue(std::string str);
		std::string		_decodeUri(const std::string& uri);
		std::string		_sanitizeUri(const std::string& uri);
		bool			_checkHeader(const std::string& str);
		void			_setLanguage();
		bool			_isChunkEnd();
		size_t			_fullRequestReceived();
		int				_parseChunkedRequest(std::string& unprocessedBuffer,
										const char* buffer);

		bool			_strIsUpper(const std::string& str);
		bool			_isCtlCharacter(int c);
		bool			_isNotCtlString(const std::string& s);

		bool	_loadServerConfig(const server_configs& serverConfigs);
		bool	_loadLocation(const ServerConfig& serverConfig);
		bool	_loadExtensionLocation(const ServerConfig& serverConf);
		bool	_loadExtensionLocation(const Location& location);
		int		_checkHeaders() const;
		int		_checkHost() const;
		int		_checkMaxBodySize() const;
		int		_checkMethod() const;
	
		bool	_methodIsAllowed() const;

		bool	_parseRequestTarget(const std::string& requestTarget);
		void	_parseInternalTarget(const std::string& redirectTo);

		int		_parseNoCLen(std::string& unprocessedBuffer, size_t lfpos,
							const char* RecvBuffer);
		int		_parseWithCLen(std::string& unprocessedBuffer, size_t lfpos);
		int		_checkIfRequestEnded(const server_configs& serverConfigs);

		int		_generateTmpFile();
		void	_closeTmpFile();
		void	_deleteTmpFile();
	
		void	_printRequestInfo();
		void	_logError(const char* errorAt) const;

		// TO DO: Initialize them, and reset them in clearRequest()
		header_map							_headers;
		std::string							_body;
		std::string							_buffer;
		std::string							_method;
		std::string::size_type				_bufferIndex;

		const AcceptSocket&					_clientSocket;
		const ServerConfig*					_serverConfig;
		const Location*						_location;
		int									_code;
		std::string							_version;
		std::map<double, std::string>		_languages;

		// TO DO: 1) If the request line is invalid, immediately return 400
		// but record it here anyway for the debug messages
		// 2) If the request line is > 8192, immediately return 414
		std::string			_requestLine;

		// TO DO: 1) The uppercase letters, '_' and '-' are valid characters
		// that must be accepted when parsing the method in the request line,
		// that will lead to unallowed methods (like "_GET" or "P-OST")
		// 2) If invalid return 400, if not allowed _checkHeaders will return 405,
		// and if exist but is not implemented can return 501
		std::string			_requestMethod;

		// TO DO: 1) It is what comes before '#', or the first '?' starting the query,
		// and after the potential valid full scheme, domain name and port
		// 2) If the requested origin-form URI has no '/' prefix, or if it goes up
		// in directories (with "/..") such that it would go above "/"
		// (like '/../abc'), it should return a 400 error (but '/abc/../cde',
		// '/././.', '/...' are all fine)
		// 3) "/." becomes '/', "//" -> '/', "%4d" -> 'M', "%4E" -> 'N', etc...
		// All the many other rules are in the RFCs (9112 et 3986) and in the
		// NGINX implementation (https://bit.ly/3XEvVs1)
		std::string			_uri;
		std::string			_raw_uri;

		// TO DO: It is what comes after the first '?' in the URI
		std::string			_query;

		// TO DO: It is what comes after the last '.' in the URI part, but only
		// if it is not part of "/.." that goes up in a directory, if it is
		// preceded and follwed by a valid usual character
		// It is used to set the Response's "Content-Type"
		std::string			_extension;

		std::string			_contentType;

		// TO DO: Cannot be neither undefined nor empty
		std::string			_host;

		// TO DO: 1) Ignore 'Keep-Alive' Header and do not implement "timeout"
		// and "max" parameters: "HTTP/1.1 does not define any parameters"
		// 2) Only set it to false if 'Connection' Header is set to "close"
		bool				_isKeepAlive;
		bool				_hasReceivedHeaders;
		bool				_hasReceivedBody;
		bool				_hasBody;
		bool				_waitNextRequest;

		std::ofstream		_tmpFileStream;
		std::ifstream		_requestFileStream;
		// TO DO: The Content-Length also limits the size of what is actually
		// going to be processed from the body (even if it is longer)
		// 2) If the request method is DELETE and the request has a body, or
		// if it has a Content-Length header with a positive value, immediately
		// return 415 and LOG_ERROR("DELETE with body is unsupported")
		// 3) If both the "Transfer-Encoding: chunked" and "Content-Length" are
		// present, immediately return 400
		// 4) If receiving a correct chunked request, when the request is over,
		// we set the _bodySize (even if 0)
		int64_t				_bodySize;

		bool				_isChunkedRequest;

		// TO DO: 1) For POST requests, if the size is known in advance, we must
		// reserve the size in "_tmpString" (with a try-catch). If it fails, we
		// directly save the body in the "_tmpFile". If it succeeds, we first
		// save it in the "_tmpString", and if no errors, we then write it in
		// one single operation in the "_tmpFile".
		// If the size is not known in advance (or if the reserve failed), we
		// save the body in the "_tmpFile"
		// 2) The "_tmpFile" must be open in "std::ios::out | std::ios::binary"
		// mode, closed when the writing is finished, AND if the body is empty
		// (without error), there must be an empty tmp file on the disk!
		std::string			_tmpString;
		std::fstream		_tmpFile;
		// TO DO: Create a unique name
		std::string			_tmpFilename;

		bool				_isTerminatedRequest;
		bool				_isInternalRedirect;
	};

}	// namespace webserv

#endif	// REQUEST_HPP

