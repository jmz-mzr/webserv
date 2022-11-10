#include "core/AcceptSocket.hpp"

#include <cstring>
#include <cerrno>

#include <fcntl.h>

#include "utils/Logger.hpp"
#include "webserv_config.hpp"
#include "utils/utils.hpp"

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
			LOG_DEBUG("accept() error: " << strerror(errno));
			throw FatalErrorException("Fatal error on accept() call");
		}
		if (MACOS)
			fcntl(_fd, F_SETFL, O_NONBLOCK);
		_port = ntohs(_addr.sin_port);
		_ipAddr = ft_inet_ntoa(_addr.sin_addr);
		setsockopt(_fd, SOL_SOCKET, SO_SNDBUF, &sendBufferSize, sizeof(int));
		LOG_INFO("New accepted socket");
		LOG_DEBUG("fd=" << _fd << " ; addr=" << _ipAddr << " ; port=" << _port);
	}

}	// namespace webserv
