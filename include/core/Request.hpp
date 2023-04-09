#ifndef REQUEST_HPP
# define REQUEST_HPP

# include <stddef.h>	// size_t
# include <stdint.h>	// int64_t

# include <fstream>
# include <map>
# include <string>
# include <vector>

# include "config/ServerConfig.hpp"
# include "config/Location.hpp"
# include "core/AcceptSocket.hpp"
# include "utils/utils.hpp"

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

		const AcceptSocket&		getClientSocket() const
										{ return (_clientSocket); }
		const ServerConfig*		getServerConfig() const
										{ return (_serverConfig); }
		const std::string&		getServerName() const;
		const Location*			getLocation() const
										{ return (_location); }
		const std::string&		getRequestLine() const
										{ return (_requestLine); }
		const std::string&		getRequestMethod() const
										{ return (_method); }
		const std::string&		getUri() const
										{ return (_uri); }
		const std::string&		getQuery() const
										{ return (_query); }
		const std::string&		getExtension() const
										{ return (_extension); }
		const header_map&		getHeaders() const
										{ return (_headers); }
		const std::string&		getHost() const
										{ return (_host); }
		int64_t					getContentLength() const
										{ return (_contentLength); }
		const std::string&		getContentType() const;
		std::fstream&			getTmpFile()
										{ return (_tmpFile); }
		const std::string&		getTmpFilename() const
										{ return (_tmpFilename); }

		bool	isKeepAlive() const;
		bool	isChunkedRequest() const { return (_isChunkedRequest); }
		bool	isTerminatedRequest() const { return (_isTerminatedRequest); }
		bool	isInternalRedirect() const { return (_isInternalRedirect); }

		int		parseRequest(const std::string& recvBuffer,
								const server_configs& serverConfigs);

		void	setRequestMethod(const std::string& method);
		int		loadInternalRedirect(const std::string& redirectTo,
										bool reloadLocation = true);

		void	clearRequest();
	private:
		typedef ServerConfig::location_map		_location_map;
		typedef std::map<double, std::string>	_weight_map;

		Request();

		Request&	operator=(const Request& rhs);

		std::string 	_readLine(bool allowNonPrintable = true);

		bool		_parseStartAndFieldLines();
		bool		_parseRequestLine(const std::string& line);
		size_t		_parseMethod(const char* str);
		size_t		_parseOriginUri(const char* str);
		size_t		_parseAbsoluteUri(const char* str);
		size_t		_parseUriHost(const char* str);
		size_t		_parseUriPath(const char* str);
		size_t		_parseUriInfos(const char* str);
		bool		_decodeUri();
		size_t		_handleDotSegment(size_t i, std::string& uri) const;
		size_t		_unescapePctChar(size_t i, std::string& uri) const;
		bool		_logUri();
		size_t		_parseHttpVersion(const char* str);

		bool	_parseHeaderLine(const std::string& line);
		bool	_loadHeaderFields(const std::string& line,
									std::string& fieldName,
									std::string& fieldValue);
		bool	_isHeaderName(const char c) const;
		bool	_loadHeaderValue(const std::string& line,
									std::string::const_iterator c,
									std::string& fieldValue);
		bool	_recordHeader(const std::string& fieldName,
								const std::string& fieldValue);
		bool	_recordHostHeader(const std::string& fieldValue);
		bool	_recordConnectionHeader(const std::string& fieldValue);
		bool	_recordAcceptLanguageHeader(const std::string& fieldValue);
		bool	_recordGenericHeader(const std::string& fieldName,
										const std::string& fieldValue,
										bool mustBeUnique = false);
		bool	_recordOtherHeader(const std::string& fieldName,
									const std::string& fieldValue);
		bool	_checkHost();
		bool	_checkContentLength();
		bool	_checkTransferEncoding();

		bool	_loadServerConfig(const server_configs& serverConfigs);
		bool	_loadLocation(const ServerConfig& serverConfig);
		bool	_loadExtensionLocation(const ServerConfig& serverConf);
		bool	_loadExtensionLocation(const Location& location);
		bool	_ignoreLocation(const Location& location) const;
		bool	_checkConfig();
		bool	_checkMaxBodySize();
		bool	_checkMethod();
		bool	_checkReturn();

		void	_discardBody();
		void	_discardChunkedBody();
		bool	_discardChunk();
		bool	_discardChunkedTrailer();

		bool	_createBodyTmpFile();
		bool	_writeBodyTmpFile(int64_t n);

		int		_parseChunkedRequest();
		bool	_parseChunkSize(const std::string& line);
		size_t	_convertChunkSize(const std::string& line);
		bool	_checkChunkExt(const std::string& line, size_t i);
		bool	_parseChunkTrailer(const std::string& line);
		bool	_loadChunk();

		bool	_loadBody();

		void	_parseInternalTarget(const std::string& redirectTo);

		void	_printRequestInfo() const;
		void	_logError(const char* errorAt) const;

		void	_clearBuffer(bool clearLastEmptyLines = false);
		void	_clearStartAndFieldLines();
		void	_closeTmpFile();
		void	_deleteTmpFile();

		const AcceptSocket&		_clientSocket;
		const ServerConfig*		_serverConfig;
		const Location*			_location;
		std::string				_buffer;
		size_t					_bufferIndex;
		std::string				_requestLine;
		std::string				_method;
		std::string				_uri;
		std::string				_query;
		std::string				_extension;
		std::string				_httpVersion;
		int						_errorCode;
		header_map				_headers;
		std::string				_host;
		int64_t					_contentLength;
		int64_t					_bodySize;
		bool					_isKeepAlive;
		_weight_map				_languages;
		bool					_isChunkedRequest;
		bool					_hasReceivedHeaders;
		std::string				_tmpFilename;
		std::fstream			_tmpFile;
		bool					_isTerminatedRequest;
		bool					_isInternalRedirect;
	};

}	// namespace webserv

#endif	// REQUEST_HPP
