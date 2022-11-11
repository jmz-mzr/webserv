#ifndef RESPONSE_HPP
# define RESPONSE_HPP

# include "core/Request.hpp"

# include <string>

namespace	webserv
{

	class	Response {
	public:
		Response();
		Response(const Response& src);
		~Response() { }	// clear _requestedFile if not automatic

		const std::string&	getResponseBuffer() const;
		int					getResponseCode() const { return (_responseCode); }

		bool	isResponseReady() const { return (_isResponseReady); }
		bool	isChunkedResponse() const { return (_isChunkedResponse); }

		void	setResponseCode(int responseCode);
		void	prepareResponse(const Request& request);
		void	prepareErrorResponse(int errorCode = 0);

		void	clearResponse();
	private:
		Response&	operator=(const Response& rhs);

		void	_prepareChunkedResponse(const Request& request);

		std::string		_responseBuffer;
//		std::ifstream	_requestedFile;
		int				_responseCode;
		bool			_isChunkedResponse;
		bool			_isResponseReady;
	};

}	// namespace webserv

#endif	// RESPONSE_HPP
