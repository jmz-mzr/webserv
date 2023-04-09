#include <ctime>	// difftime, time

#include <exception>

#include "core/Client.hpp"
#include "utils/log.hpp"

namespace	webserv
{

	/**************************************************************************/
	/*                       CONSTRUCTORS / DESTRUCTORS                       */
	/**************************************************************************/

	Client::Client(const int serverFd, const server_configs& serverConfigs):
												_serverFd(serverFd),
												_serverConfigs(serverConfigs),
												_socket(serverFd),
												_lastUpdateTime(std::time(0)),
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
										_lastUpdateTime(std::time(0)),
										_request(_socket)
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

	int	Client::parseRequest(const std::string& recvBuffer)
	{
		int		errorCode;

		try {
			errorCode = _request.parseRequest(recvBuffer, _serverConfigs);
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

	bool	Client::hasTimedOut() const
	{
		double	elapsedTime = std::difftime(std::time(0), _lastUpdateTime);

		if (elapsedTime > CLIENT_TIMEOUT) {
			LOG_DEBUG("The client (fd=" << _socket.getFd() << ") timed out"
					<< " (" << elapsedTime << "s)");
			return (true);
		}
		return (false);
	}

	bool	Client::prepareResponse()
	{
		try {
			_response.prepareResponse(_request);
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
		} catch (const std::exception& e) {
			_logError("Unable to prepare the error response", e.what());
			return (false);
		}
		return (true);
	}

	bool	Client::sendResponse(int ioFlags)
	{
		int		clientFd = getSocket().getFd();

		updateTimeout();
		return (_response.sendResponse(_request, clientFd, ioFlags));
	}

}	// namespace webserv
