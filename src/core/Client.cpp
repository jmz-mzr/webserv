#include "core/Client.hpp"
#include "utils/Logger.hpp"

#include <exception>

namespace	webserv
{

	/**************************************************************************/
	/*                       CONSTRUCTORS / DESTRUCTORS                       */
	/**************************************************************************/

	Client::Client(const int serverFd, const server_configs& serverConfigs):
												_serverFd(serverFd),
												_serverConfigs(serverConfigs),
												_socket(serverFd),
//												_isKeepAlive(true),
												_request(_socket)
	{
		LOG_INFO("New Client instance");
		LOG_DEBUG("serverFd=" << _serverFd << " ; "
				<< "fd=" << _socket.getFd() << " ; "
				<< "addr=" << _socket.getIpAddr() << " ; "
				<< "port=" << _socket.getPort());
	}

	Client::Client(const Client& src): _serverFd(src._serverFd),
										_serverConfigs(src._serverConfigs),
										_socket(src._socket),
//										_isKeepAlive(true),
										_request(src._request)
	{
		// NOTE: Except at Client creation (inserted in the client list),
		// the Client should not be copied

		LOG_INFO("Client copied");
		LOG_DEBUG("serverFd=" << _serverFd << " ; "
				<< "fd=" << _socket.getFd() << " ; "
				<< "addr=" << _socket.getIpAddr() << " ; "
				<< "port=" << _socket.getPort());
	}

	/**************************************************************************/
	/*                            MEMBER FUNCTIONS                            */
	/**************************************************************************/

	void	Client::_logError(const char* errorAt,
								const char* errorType) const
	{
		LOG_ERROR(errorAt << ": " << errorType);
		if (_request.getLocation()) {
			LOG_DEBUG("client: " << _socket.getIpAddr()
					<< ", client fd: " << _socket.getFd()
					<< ", server: " << _request.getServerName()
					<< ", server fd: " << _serverFd
					<< ", request: \"" << _request.getRequestLine() << "\""
					<< ", host: \"" << _request.getHost() << "\"");
		} else
			LOG_DEBUG("client: " << _socket.getIpAddr()
					<< ", client fd: " << _socket.getFd()
					<< ", server fd: " << _serverFd
					<< ", request: \"" << _request.getRequestLine() << "\"");
	}

	int	Client::parseRequest(const char* recvBuffer)
	{
		int		errorCode;

		try {
			errorCode = _request.parseRequest(_unprocessedBuffer, recvBuffer,
													_serverConfigs);
		} catch (const std::exception& e) {
			_logError("Unable to parse the client request", e.what());
			errorCode = 500;
		}
		return (errorCode);
	}

	bool	Client::hasUnprocessedBuffer() const
	{
		return (!_unprocessedBuffer.empty());
	}

	bool	Client::isProcessingRequest() const
	{
		if (hasRequestTerminated() || hasResponseReady())
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

	bool	Client::isKeepAlive() const
	{
		// TO DO: Implement a timeout: setResponseCode(408) and return (false)
		// si timeout pour ne pas clutter le server?
		// If so, either implement a timeout directive, or a default timeout
		// and update it when sending parts of a partial response

		return (true);
	}

	bool	Client::prepareResponse()
	{
		try {
			_response.prepareResponse(_request);
//			if (!_response.isPartialResponse())
//				_request.clearRequest();	// not here?
		} catch (const std::exception& e) {
			_logError("Unable to prepare the request's response", e.what());
			return (false);
		}
		return (true);
	}

	bool	Client::prepareErrorResponse(int errorCode)
	{
		try {
			_response.prepareErrorResponse(_request, errorCode);
//			_request.clearRequest();	// not here?
		} catch (const std::exception& e) {
			_logError("Unable to prepare the error response", e.what());
			return (false);
		}
		return (true);
	}

	bool	Client::sendResponse(int ioFlags)
	{
		int		clientFd = getSocket().getFd();

		return (_response.sendResponse(_request, clientFd, ioFlags));
	}

}	// namespace webserv
