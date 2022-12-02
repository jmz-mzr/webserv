#include "core/AcceptSocket.hpp"

#include <fcntl.h>

#include "utils/exceptions.hpp"
#include "utils/Logger.hpp"
#include "webserv_config.hpp"

namespace	webserv
{

	/**************************************************************************/
	/*                       CONSTRUCTORS / DESTRUCTORS                       */
	/**************************************************************************/

	AcceptSocket::AcceptSocket(const int fdListened): Socket(kAccept)
	{
		const int	sendBufferSize = SEND_BUFFER_SIZE;

		_fd = accept(fdListened,
				reinterpret_cast<struct sockaddr*>(&_addr), &_addrLen);
		if (_fd < 0) {
			throw FatalErrorException("accept() error: ", errno);
		}
#ifdef MACOS
		fcntl(_fd, F_SETFL, O_NONBLOCK);
#endif
		_port = ntohs(_addr.sin_port);
		_ipAddr = ft_inet_ntoa(_addr.sin_addr);
		setsockopt(_fd, SOL_SOCKET, SO_SNDBUF, &sendBufferSize, sizeof(int));
		LOG_INFO("New accepted socket");
		LOG_DEBUG("fd=" << _fd << " ; addr=" << _ipAddr << " ; port=" << _port);
	}

}	// namespace webserv
