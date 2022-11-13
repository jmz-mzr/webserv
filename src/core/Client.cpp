#include "core/Client.hpp"
#include "utils/Logger.hpp"

namespace	webserv
{

	/**************************************************************************/
	/*                       CONSTRUCTORS / DESTRUCTORS                       */
	/**************************************************************************/

	Client::Client(const int id, const int serverFd,
					const server_configs& serverConfigs): _id(id),
												_serverFd(serverFd),
												_serverConfigs(serverConfigs),
												_socket(serverFd),
												_isKeepAlive(true)
	{
		LOG_INFO("New Client instance");
		LOG_DEBUG("id=" << _id << " ; " << "serverFd=" << _serverFd << " ; "
				<< "fd=" << _socket.getFd() << " ; "
				<< "addr=" << _socket.getIpAddr() << " ; "
				<< "port=" << _socket.getPort());
	}

	Client::Client(const Client& src): _id(src._id), _serverFd(src._serverFd),
										_serverConfigs(src._serverConfigs),
										_socket(src._socket),
										_isKeepAlive(src._isKeepAlive),
										_request(src._request),
										_response(src._response)
	{
		LOG_INFO("Client copied");
		LOG_DEBUG("id=" << _id << " ; " << "serverFd=" << _serverFd << " ; "
				<< "fd=" << _socket.getFd() << " ; "
				<< "addr=" << _socket.getIpAddr() << " ; "
				<< "port=" << _socket.getPort());
	}

	/**************************************************************************/
	/*                            MEMBER FUNCTIONS                            */
	/**************************************************************************/

	void	Client::parseRequest(const char* buffer)
	{
		int		responseCode = _request.parseRequest(buffer, _serverConfigs);

		if (responseCode >= 400 && responseCode <= 599)
			_response.setResponseCode(responseCode);
	}

	bool	Client::isProcessingRequest() const
	{
		if (_request.isChunkedRequest()
				|| hasRequestTerminated() || hasResponseReady())
			return (true);
		return (false);
	}

	bool	Client::hasError() const
	{
		int		responseCode = _response.getResponseCode();

		if (responseCode >= 400 && responseCode <= 599)
			return (true);
		return (false);
	}

	bool	Client::hasRequestTerminated() const
	{
		return (_request.isTerminatedRequest());
	}

	bool	Client::hasResponseReady() const
	{
		return (_response.isResponseReady());
	}

	void	Client::prepareResponse()
	{
		// TO DO: If implemented, compute keepalive parameters and update
		// 		  the client, request and/or response accordingly
		// 		  Also update it relative to the error codes (like 400)

		_response.prepareResponse(_request);
		if (!_response.isChunkedResponse())
			_request.clearRequest();
	}

	void	Client::prepareErrorResponse(int errorCode)
	{
		// TO DO: If implemented, compute keepalive parameters and update
		// 		  the client, request and/or response accordingly
		// 		  Also update it relative to the error codes (like 400)

		_response.prepareErrorResponse(errorCode);
		_request.clearRequest();
	}

}	// namespace webserv
