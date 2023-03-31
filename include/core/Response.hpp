#ifndef RESPONSE_HPP
# define RESPONSE_HPP

# include <stddef.h>	// size_t
# include <stdint.h>	// int64_t
# include <stdio.h>		// FILE
# include <dirent.h>	// DIR
# include <sys/stat.h>	// struct stat

# include <ctime>		// time_t

# include <fstream>
# include <set>
# include <string>
# include <utility>		// pair

# include "core/Request.hpp"
# include "core/CgiHandler.hpp"
# include "utils/utils.hpp"

namespace	webserv
{

	class	Response {
	public:
		Response();
		Response(const Response& src);
		~Response();

		static void		initResponseMaps();

		const std::string&	getResponseBuffer() const;
		int					getResponseCode() const { return (_responseCode); }

		bool	isKeepAlive(const Request& request) const;
		bool	isResponseReady() const { return (_isResponseReady); }
		bool	isPartialResponse() const;

		void	prepareResponse(Request& request);
		void	prepareErrorResponse(Request& request, int errorCode = 0);

		bool	sendResponse(const Request& request, int clientFd, int ioFlags);

		void	clearResponse(const Request& request,
								int responseCodeToKeep = 0);
	private:
		typedef std::pair<std::string, struct stat>				_dir_entry_pair;
		typedef std::set<_dir_entry_pair, dir_entry_compare>	_dir_entry_set;

		Response&	operator=(const Response& rhs);

		bool	_loadReturn(const Request& request);
		void	_loadRelativeLocationPrefix(const Request& request);
		bool	_loadErrorPage(Request& request);

		int				_loadRequestedFile(Request& request);
		bool			_loadFileWithAlias(Request& request);
		int				_openRequestedFile(const Request& request);
		bool			_openAndStatFile(const Request& request,
											struct stat* fileInfos);
		void			_closeRequestedFile();
		void			_loadDirLocation(const Request& request);
		void			_loadFileHeaders(const struct stat* fileInfos);
		std::string		_getFileExtension();
		int				_checkConditionalHeaders(const Request& request,
											const struct stat* fileInfos) const;
		int				_checkIfMatch(const Request& request,
										const struct stat* fileInfos,
										bool ifNoneMatch = false) const;
		bool			_findMatch(const std::string& value,
									const std::string& eTag) const;
		int				_checkIfModifiedSince(const Request& request,
										const struct stat* fileInfos,
										bool ifUnmodifiedSince = false) const;
		time_t			_parseTime(const char* timeStr) const;

		int		_loadIndex(Request& request);
		int		_loadInternalRedirect(Request& request,
										const std::string& redirectTo);
		int		_loadDirIndex(const Request& request, size_t indexLen);
		int		_loadAutoIndex(const Request& request);

		void	_prepareFileResponse(Request& request);
		void	_prepareChunkedResponse(Request& request);

		int				_loadDirEntries(const Request& request);
		int				_loadDirEntry(const Request& request,
										const char* entryName);
		void			_closeIndexDirectory();
		void			_loadAutoindexHtml(const Request& request);
		void			_loadAutoindexEntry(const _dir_entry_pair& entry);
		std::string		_escapeHtml(const std::string& str,
									size_t maxLen = std::string::npos) const;
		std::string		_escapeUriComponent(const std::string& uri) const;
		std::string		_escapeUri(const std::string& uri) const;
		std::string		_getFileDate(const struct stat& fileInfos) const;
		std::string		_getFileSize(const struct stat& fileInfos) const;
		void			_loadChunkHeaderAndTrailer(bool isLastChunk);

		int		_removeRequestedFile(const Request& request);
		int		_removeRequestedDirectory(const Request& request);
		bool	_removeDirectoryTree(const Request& request, const char* path);
		bool	_removeDirEntry(const Request& request, const char* dirPath,
								const char* entryName, bool* hasError);
		void	_deleteFile(const Request& request, const char* filename);
		void	_deleteDirectory(const Request& request, const char* dirname);
		int 	_unlinkFile(const Request& request);

		int				_putRequestBody(const Request& request);
		int				_moveRequestTmpFile(const Request& request,
											const struct stat* fileInfos,
											bool fileExists = false);
		int				_handleAlreadyExistingFile(const Request& request,
												const struct stat* fileInfos);
		int				_postRequestBody(Request& request);
		int				_createPostFile(Request& request);
		std::string		_createPostFilename(const Request& request,
												bool addSlash) const;
		void			_setPutPostHeaders(const Request& request);
		std::string		_getPutPostResponseBody(const Request& request);

		int		_loadCgiPass(Request& request);
		int		_statCgiScript(const Request& request);
		int		_processCgiOutput(Request& request, CgiHandler& cgi);
		void	_prepareCgiOutputParsing(FILE* cgiOutputFile);
		int		_readCgiHeaders(const Request& request, CgiHandler& cgi,
								char* buffer);
		bool	_parseCgiHeader(const Request& request, CgiHandler& cgi,
								const char* buffer);
		bool	_loadCgiHeaderFields(const Request& request, const char* buffer,
										std::string& fieldName,
										std::string& fieldValue) const;
		bool	_isFieldName(const char c) const;
		bool	_isFieldValue(const char c) const;
		bool	_recordCgiHeader(const Request& request, CgiHandler& cgi,
									const std::string& fieldName,
									const std::string& fieldValue,
									const char* buffer);
		bool	_parseCgiContentType(const Request& request,
										const std::string& fieldValue);
		bool	_parseCgiLocation(const Request& request,
									const std::string& fieldValue);
		bool	_parseCgiLocalLocation(const Request& request,
										const std::string& fieldValue);
		bool	_checkCgiClientLocation(const std::string& fieldValue);
		bool	_parseCgiStatus(const Request& request, CgiHandler& cgi,
								const std::string& fieldValue) const;
		bool	_parseCgiContentLength(const Request& request,
										const std::string& fieldValue);
		bool	_checkCgiHeaders(const Request& request, CgiHandler& cgi);
		bool	_checkCgiEof(FILE* cgiOutputFile) const;
		bool	_checkCgiLocation(const Request& request,
									CgiHandler& cgi) const;
		bool	_loadTmpCgiBodyFile(const Request& request,
									FILE* cgiOutputFile);
		bool	_createTmpCgiBodyFile(const Request& request);
		bool	_checkCgiBodyLength(const Request& request,
									const char* filename);
		void	_loadCgiHeaders(const Request& request, CgiHandler& cgi);

		bool			_handleBodyDrop(const Request& request);
		void			_loadHeaders(const Request& request);
		std::string		_getAllowedMethods(const Request& request) const;
		std::string		_getETag(const struct stat* fileInfos = 0) const;

		void	_clearBuffer();
		void	_clearBuffersAndFiles(const Request& request);
		void	_deleteTmpCgiBodyFile(const Request& request);

		void	_logError(const Request& request, const char* errorAt,
						const char* errorType, const char* filename = 0) const;

		static const std::string&	_getDate(time_t lastModifiedTime = -1);
		static const std::string&	_getResponseStatus(int responseCode);
		static const std::string&	_getSpecialResponseBody(int responseCode);
		static const std::string&	_getContentType(const std::string&
																	extension);

		std::string		_responseBuffer;
		std::string		_requestedFilename;
		std::string		_tmpCgiBodyFilename;
//		int				_requestedFileFd;
		std::fstream	_requestedFile;
		char*			_fileBuffer;
		size_t			_fileBufferSize;
		size_t			_bufferPos;
		DIR*			_indexDirectory;
		_dir_entry_set	_dirEntrySet;
		int				_responseCode;
		std::string		_contentType;
		int64_t			_contentLength;
		time_t			_lastModifiedTime;
		bool			_isKeepAlive;
		std::string		_location;
		bool			_isChunkedResponse;
		bool			_isFileResponse;
		bool			_isResponseReady;
	};

}	// namespace webserv

#endif	// RESPONSE_HPP
