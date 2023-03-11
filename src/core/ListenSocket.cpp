#include <sys/socket.h>		// bind, listen, setsockopt, struct sockaddr

#include <cerrno>			// errno
#include <cstring>			// strerror

#include "core/ListenSocket.hpp"
#include "utils/log.hpp"
#include "utils/exceptions.hpp"
#include "utils/utils.hpp"

namespace	webserv
{

	/**************************************************************************/
	/*                       CONSTRUCTORS / DESTRUCTORS                       */
	/**************************************************************************/

	ListenSocket::ListenSocket(const sockaddr_in& address)
			: Socket(kListen, address)
	{
		const int enable = 1;
		setsockopt(_fd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int));
		if (bind(_fd,
					reinterpret_cast<struct sockaddr*>(&_addr), _addrLen) < 0) {
			closeFd();
			LOG_DEBUG(*this);
			THROW_FATAL("bind() error: " << strerror(errno));
		}
		if (listen(_fd, _kListenBacklog) < 0) {
			closeFd();
			LOG_DEBUG(*this);
			THROW_FATAL("listen() error: " << strerror(errno));
		}
		LOG_DEBUG(*this);
	}

}	// namespace webserv
