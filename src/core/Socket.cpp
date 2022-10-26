#include "core/Socket.hpp"

#include <unistd.h>

#include "utils/utils.hpp"
#include "utils/Logger.hpp"

namespace webserv {

Socket::Socket() : type(kAny), fd(-1), addr_len(sizeof(addr))
{ ft_memset(&addr, 0, addr_len); }

Socket::~Socket()
{
	if (fd >= 0) {
		if (close(fd) < 0) {
			LOG_WARN("Bad close() on fd=" << fd);
		} else {
			LOG_INFO("close(" << fd << ")");
		}
	}
}

Socket::Socket(enum SocketType t) : type(t), fd(-1), addr_len(sizeof(addr))
{ ft_memset(&addr, 0, addr_len); }

Socket::Socket(enum SocketType t,
				int filedes,
				struct sockaddr_in address,
				socklen_t address_len)
		: type(t), fd(filedes), addr(address), addr_len(address_len)
{ }

Socket::Socket(const Socket& src) : type(src.type), fd(src.fd), addr(src.addr)
{ }

Socket&	Socket::operator=(const Socket& rhs)
{
	fd = rhs.fd;
	addr = rhs.addr;
	return *this;
}

const int&	Socket::getFd() const { return fd; }

}	// namespace webserv