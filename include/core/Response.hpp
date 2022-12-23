#ifndef RESPONSE_HPP
# define RESPONSE_HPP

# include <string>

# include "core/Request.hpp"

namespace	webserv
{

	class	Response {
	public:
		Response();
		Response(const Response& src);
		~Response() { }	// clear _requestedFile if not automatic

		const std::string&	getResponseBuffer() const;
		int					getResponseCode() const { return (_responseCode); }

		bool	isKeepAlive() const;
		bool	isResponseReady() const { return (_isResponseReady); }
		bool	isChunkedResponse() const { return (_isChunkedResponse); }

		void	setResponseCode(int responseCode);
		void	prepareResponse(Request& request);
		void	prepareErrorResponse(Request& request, int errorCode = 0);

		void	clearResponse(int responseCodeToKeep = 0);
	private:
		Response&	operator=(const Response& rhs);

		bool				_loadErrorPage(Request& request);
		void				_prepareChunkedResponse(Request& request);
		void				_loadHeaders();
		const std::string	_loadLocation();

		static const std::string&	_getResponseStatus(int responseCode);
		static const std::string&	_getDate();
		static const std::string&	_getSpecialResponseBody(int responseCode);

		std::string		_responseBuffer;
		// TO DO: The search for the requested file must be case-insensitive
//		std::ifstream	_requestedFile;
		int				_responseCode;
		// TO DO: Becomes "text/html" when _autoIndex, _specialResponseBody
		std::string		_contentType;
		int64_t		_contentLength;
		bool			_isKeepAlive;
		// The "Location" header if (201 if created?)/301/302/303/307/308
		std::string		_location;
		bool			_isChunkedResponse;
		bool			_isResponseReady;
	};

}	// namespace webserv

#endif	// RESPONSE_HPP
