#include "core/Client.hpp"
#include "utils/Logger.hpp"

namespace	webserv
{

	/**************************************************************************/
	/*                       CONSTRUCTORS / DESTRUCTORS                       */
	/**************************************************************************/

	Client::Client(const int id, const int serverFd): _id(id),
													_socket(serverFd)
	{
		LOG_INFO("New Client instance");
		LOG_DEBUG("id=" << _id << " ; " << "fd=" << _socket.getFd() << " ; "
				<< "addr=" << _socket.getIpAddr() << " ; "
				<< "port=" << _socket.getPort());
	}

	Client::Client(const Client& src): _id(src._id), _socket(src._socket),
										_request(src._request),
										_response(src._response)
	{
		LOG_INFO("Client copied");
		LOG_DEBUG("id=" << _id << " ; " << "fd=" << _socket.getFd() << " ; "
				<< "addr=" << _socket.getIpAddr() << " ; "
				<< "port=" << _socket.getPort());
	}

	/**************************************************************************/
	/*                            MEMBER FUNCTIONS                            */
	/**************************************************************************/

	void	Client::parseRequest(const char* buffer)
	{
		int		responseCode = _request.parseRequest(buffer);

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
		_response.prepareResponse(_request);
		if (!_response.isChunkedResponse())
			_request.clearRequest();
	}

	void	Client::prepareErrorResponse(int errorCode)
	{
		_request.clearRequest();
		_response.prepareErrorResponse(errorCode);
	}

}	// namespace webserv
