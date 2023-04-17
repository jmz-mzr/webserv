#include "core/AcceptSocket.hpp"

#include <arpa/inet.h>		// ntohs
#include <fcntl.h>			// fcntl
#include <sys/socket.h>		// accept, setsockopt, struct sockaddr

#include <cerrno>			// errno
#include <cstring>			// strerror

#include "utils/exceptions.hpp"
#include "utils/log.hpp"
#include "utils/utils.hpp"
#include "webserv_config.hpp"

namespace	webserv
{

	/**************************************************************************/
	/*                       CONSTRUCTORS / DESTRUCTORS                       */
	/**************************************************************************/

	AcceptSocket::AcceptSocket(const int fdListened): Socket(kAccept)
	{
		const int	sendBufferSize = SEND_BUFFER_SIZE;
		int			options = SO_SNDBUF;

		_fd = accept(fdListened,
				reinterpret_cast<struct sockaddr*>(&_addr), &_addrLen);
		if (_fd < 0) {
			LOG_DEBUG("ip=" << _ip << " port=" << _port);
			THROW_FATAL("accept() error: " << std::strerror(errno));
		}
#ifdef MACOS
		fcntl(_fd, F_SETFL, O_NONBLOCK);
		options |= SO_NOSIGPIPE;
#endif
		_port = ntohs(_addr.sin_port);
		_ip = ft_inet_ntoa(_addr.sin_addr);
		setsockopt(_fd, SOL_SOCKET, options, &sendBufferSize, sizeof(int));
		LOG_INFO("New accepted socket");
		LOG_DEBUG("fd=" << _fd << " ; addr=" << _ip << " ; port=" << _port);
	}

}	// namespace webserv
