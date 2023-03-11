#include "core/ConnectSocket.hpp"

#include <sys/socket.h>		// connect, struct sockaddr

#include <cerrno>			// errno
#include <cstring>			// strerror

#include "utils/exceptions.hpp"
#include "utils/log.hpp"
#include "utils/utils.hpp"

namespace	webserv
{

	/**************************************************************************/
	/*                       CONSTRUCTORS / DESTRUCTORS                       */
	/**************************************************************************/

	ConnectSocket::ConnectSocket(const sockaddr_in& address)
			: Socket(kConnect, address)
	{
		if (connect(_fd,
					reinterpret_cast<struct sockaddr*>(&_addr), _addrLen) < 0) {
			closeFd();
			LOG_DEBUG("ip=" << _ip << " port=" << _port);
			THROW_FATAL("connect() error: " << std::strerror(errno));
		}
		LOG_INFO("New connect socket on " << _ip << ":" << _port);
		LOG_DEBUG("fd=" << _fd);
	}

}	// namespace webserv
