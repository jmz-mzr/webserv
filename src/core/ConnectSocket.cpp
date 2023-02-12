#include "core/ConnectSocket.hpp"

#include <cerrno>
#include <cstring>

#include "utils/exceptions.hpp"
#include "utils/Logger.hpp"
#include "utils/utils.hpp"

namespace	webserv
{

	/**************************************************************************/
	/*                       CONSTRUCTORS / DESTRUCTORS                       */
	/**************************************************************************/

	ConnectSocket::ConnectSocket(const sockaddr_in& address)
			: Socket(kConnect, address)
	{
		if (connect(_fd, reinterpret_cast<sockaddr*>(&_addr), _addrLen) < 0) {
			closeFd();
			LOG_DEBUG("ip=" << _ip << " port=" << _port);
			THROW_FATAL("connect() error: " << strerror(errno));
		}
		LOG_INFO("New connect socket on " << _ip << ":" << _port);
		LOG_DEBUG("fd=" << _fd);
	}

}	// namespace webserv
