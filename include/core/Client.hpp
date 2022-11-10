#ifndef CLIENT_HPP
# define CLIENT_HPP

# include <string>

# include "core/AcceptSocket.hpp"
# include "core/Request.hpp"
# include "core/Response.hpp"

namespace	webserv
{

	class	Client {
	public:
		Client(const int id, const int serverFd);	// tmp exam version
		Client(const Client& src);
		~Client() { }

		const int&				getId() const { return (_id); }	// tmp exam version
		const AcceptSocket&		getSocket() const { return (_socket); }

		void	parseRequest(const char* buffer);

		const Request&		getRequest() const { return (_request); }
		const Response&		getResponse() const { return (_response); }

		bool	isProcessingRequest() const;
		bool	hasError() const;
		bool	hasRequestTerminated() const;
		bool	hasResponseReady() const;

		void	prepareResponse();
		void	prepareErrorResponse(int errorCode = 0);

		void	clearRequest() { _request.clearRequest(); }
		void	clearResponse() { _response.clearResponse(); }

		void	closeSocket() { _socket.closeFd(); }

		std::string		buffer;	// tmp exam version
	private:
		Client&	operator=(const Client& rhs);

		int					_id;	// tmp exam version
		AcceptSocket		_socket;
		Request				_request;
		Response			_response;
	};

}	// namespace webserv

#endif	// CLIENT_HPP