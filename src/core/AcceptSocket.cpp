#include "core/AcceptSocket.hpp"

#include <cerrno>
#include <cstring>

#include <fcntl.h>

#include "utils/exceptions.hpp"
#include "utils/log.hpp"
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
			LOG_DEBUG("ip=" << _ip << " port=" << _port);
			THROW_FATAL("accept() error: " << strerror(errno));
		}
#ifdef MACOS
		fcntl(_fd, F_SETFL, O_NONBLOCK);
#endif
		_port = ntohs(_addr.sin_port);
		_ip = ft_inet_ntoa(_addr.sin_addr);
		setsockopt(_fd, SOL_SOCKET, SO_SNDBUF, &sendBufferSize, sizeof(int));
		LOG_INFO("New accepted socket");
		LOG_DEBUG("fd=" << _fd << " ; addr=" << _ip << " ; port=" << _port);
	}

}	// namespace webserv
