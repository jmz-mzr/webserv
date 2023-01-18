#ifndef RESPONSE_HPP
# define RESPONSE_HPP

# include <sys/stat.h>
# include <sys/types.h>
# include <dirent.h>

# include <string>
# include <utility>
# include <set>
# include <ctime>

# include "core/Request.hpp"
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

		bool	isKeepAlive() const;
		bool	isResponseReady() const { return (_isResponseReady); }
		bool	isPartialResponse() const;

		void	setResponseCode(int responseCode);
		void	prepareResponse(Request& request);
		void	prepareErrorResponse(Request& request, int errorCode = 0);

		bool	sendResponse(const Request& request, int clientFd, int ioFlags);

		void	clearResponse(const Request& request,
								int responseCodeToKeep = 0);
	private:
		typedef std::pair<std::string, struct stat>				_dir_entry_pair;
		typedef std::set<_dir_entry_pair, dir_entry_compare>	_dir_entry_set;

		Response&	operator=(const Response& rhs);

		bool				_loadReturn(const Request& request);
		void				_loadRelativeLocationPrefix(const Request& request);
		bool				_loadErrorPage(Request& request);
		int					_loadRequestedFile(Request& request);
		bool				_loadFileWithAlias(Request& request);
		int					_removeRequestedFile(const Request& request);
		int					_removeRequestedDirectory(const Request& request);
		bool				_removeDirectoryTree(const Request& request,
													const char* path);
		bool				_removeDirEntry(const Request& request,
											const char* dirPath,
											const char* entryName,
											bool* hasError);
		void				_deleteFile(const Request& request,
										const char* filename);
		void				_deleteDirectory(const Request& request,
												const char* dirname);
		int					_openRequestedFile(const Request& request);
		bool				_openAndStatFile(const Request& request,
												struct stat* fileInfos);
		void				_loadDirLocation(const Request& request);
		void				_loadFileHeaders(const Request& request,
												const struct stat* fileInfos);
		void				_closeRequestedFile();
		int					_loadIndex(Request& request);
		int					_loadInternalRedirect(Request& request,
												const std::string& redirectTo);
		int					_loadDirIndex(const Request& request,
											size_t indexLen);
		int					_loadAutoIndex(const Request& request);
		void				_prepareFileResponse(Request& request);
		void				_prepareChunkedResponse(Request& request);
		void				_clearBuffer();
		int					_loadDirEntries(const Request& request);
		int					_loadDirEntry(const Request& request,
											const char* entryName);
		void				_closeIndexDirectory();
		int					_loadAutoindexHtml(const Request& request);
		void				_loadAutoindexEntry(const _dir_entry_pair& entry);
		const std::string	_escapeHtml(const std::string& str,
									size_t maxLen = std::string::npos) const;
		const std::string	_escapeUriComponent(const std::string& uri) const;
		const std::string	_escapeUri(const std::string& uri) const;
		const std::string	_getFileDate(const struct stat& fileInfos) const;
		const std::string	_getFileSize(const struct stat& fileInfos) const;
		void				_loadChunkHeaderAndTrailer(bool isLastChunk);
		void				_loadHeaders();
		const std::string	_getETag() const;

		void	_logError(const Request& request, const char* errorAt,
						const char* errorType, const char* filename = "") const;

		static const std::string&	_getDate(time_t lastModifiedTime = -1);
		static const std::string&	_getResponseStatus(int responseCode);
		static const std::string&	_getSpecialResponseBody(int responseCode);
		static const std::string&	_getContentType(const std::string&
																	extension);

		std::string		_responseBuffer;
		std::string		_requestedFileName;
//		int				_requestedFileFd;
		std::ifstream	_requestedFile;
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
