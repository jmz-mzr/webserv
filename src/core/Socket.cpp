#include "core/Socket.hpp"
#include "utils/Logger.hpp"

#include <unistd.h>

namespace	webserv
{

	/**************************************************************************/
	/*                       CONSTRUCTORS / DESTRUCTORS                       */
	/**************************************************************************/

	Socket::Socket(const Socket& src): _type(src._type), _fd(src._fd),
									_ipAddr(src._ipAddr), _port(src._port),
									_addr(src._addr), _addrLen(src._addrLen)
	{
		LOG_INFO("Socket copied");
		LOG_DEBUG("fd=" << _fd << " ; "
				<< "addr=" << _ipAddr << " ; " << "port=" << _port);
	}

	/**************************************************************************/
	/*                            MEMBER FUNCTIONS                            */
	/**************************************************************************/

	void	Socket::closeFd()
	{
		if (_fd >= 0) {
			if (close(_fd) < 0) {
				LOG_WARN("Bad close() on fd=" << _fd);
			} else {
				LOG_INFO("close(" << _fd << ")");
				_fd = -1;
			}
		}
	}

}	// namespace webserv
