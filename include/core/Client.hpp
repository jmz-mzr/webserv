#ifndef CLIENT_HPP
# define CLIENT_HPP

# include <string>
# include <vector>

# include "config/ServerConfig.hpp"
# include "core/AcceptSocket.hpp"
# include "core/Request.hpp"
# include "core/Response.hpp"

namespace	webserv
{

	class	Client {
	public:
		typedef std::vector<ServerConfig>	server_configs;

		Client(const int id, const int serverFd,
				const server_configs& serverConfigs);	// tmp exam version
		Client(const Client& src);
		~Client() { }

		const int&				getId() const { return (_id); }	// tmp exam version
		const AcceptSocket&		getSocket() const { return (_socket); }

		void	parseRequest(const char* recvBuffer);

		const Request&		getRequest() const { return (_request); }
		const Response&		getResponse() const { return (_response); }

		bool	hasError() const;
		bool	hasUnprocessedBuffer() const;
		bool	isProcessingRequest() const;
		bool	hasRequestTerminated() const;
		bool	hasResponseReady() const;
		bool	isKeepAlive() const;

		void	prepareResponse();
		void	prepareErrorResponse(int errorCode = 0);

		void	clearRequest() { _request.clearRequest(); }
		void	clearResponse() { _response.clearResponse(); }

		void	closeSocket() { _socket.closeFd(); }

		std::string		buffer;	// tmp exam version
	private:
		Client&	operator=(const Client& rhs);

		int						_id;	// tmp exam version
		int						_serverFd;
		const server_configs&	_serverConfigs;
		AcceptSocket			_socket;

		// TO DO: Rather record the time of last action (connection/interaction),
		// and check for a timeout (either by default or through directive) in
		// "isKeepAlive()" to avoid too many idle clients
//		bool					_isKeepAlive;

		std::string				_unprocessedBuffer;
		Request					_request;
		Response				_response;
	};

}	// namespace webserv

#endif	// CLIENT_HPP
