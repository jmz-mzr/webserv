#ifndef CLIENT_HPP
# define CLIENT_HPP

# include <ctime>	// time_t

# include <string>
# include <vector>

# include "config/ServerConfig.hpp"
# include "core/AcceptSocket.hpp"
# include "core/Request.hpp"
# include "core/Response.hpp"

# define CLIENT_TIMEOUT		60.0

namespace	webserv
{

	class	Client {
	public:
		typedef std::vector<ServerConfig>	server_configs;

		Client(const int serverFd, const server_configs& serverConfigs);
		Client(const Client& src);
		~Client() { }

		const AcceptSocket&		getSocket() const { return (_socket); }

		int		parseRequest(const char* recvBuffer);

		const Request&		getRequest() const { return (_request); }
		const Response&		getResponse() const { return (_response); }

		bool	hasUnprocessedBuffer() const;
		bool	isProcessingRequest() const;
		bool	hasRequestTerminated() const;
		bool	hasResponseReady() const;
		bool	hasTimedOut() const;

		void	updateTimeout() { _lastUpdateTime = std::time(0); }

		bool	prepareResponse();
		bool	prepareErrorResponse(int errorCode = 0);

		bool	sendResponse(int ioFlags);

		void	clearRequest() { _request.clearRequest(); }
		void	clearResponse() { _response.clearResponse(_request); }

		void	closeSocket() { _socket.closeFd(); }
	private:
		Client&	operator=(const Client& rhs);

		void	_logError(const char* errorAt, const char* errorType) const;

		int						_serverFd;
		const server_configs&	_serverConfigs;
		AcceptSocket			_socket;

		time_t					_lastUpdateTime;

		std::string				_unprocessedBuffer;
		Request					_request;
		Response				_response;
	};

}	// namespace webserv

#endif	// CLIENT_HPP
