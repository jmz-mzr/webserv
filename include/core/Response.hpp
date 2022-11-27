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

		bool	isKeepAlive() const { return (_isKeepAlive); }
		bool	isResponseReady() const { return (_isResponseReady); }
		bool	isChunkedResponse() const { return (_isChunkedResponse); }

		void	setResponseCode(int responseCode);
		void	prepareResponse(const Request& request);
		void	prepareErrorResponse(int errorCode = 0);

		void	clearResponse();

		static const std::string&	getSpecialResponseBody(int responseCode);
	private:
		Response&	operator=(const Response& rhs);

		void	_prepareChunkedResponse(const Request& request);

		std::string		_responseBuffer;
		// TO DO: The search for the requested file must be case-insensitive
//		std::ifstream	_requestedFile;
		int				_responseCode;
		bool			_isKeepAlive;
		bool			_isChunkedResponse;
		bool			_isResponseReady;
	};

}	// namespace webserv

#endif	// RESPONSE_HPP
