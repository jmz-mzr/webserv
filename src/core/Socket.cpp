#include "core/Socket.hpp"

#include <unistd.h>

#include "utils/utils.hpp"
#include "utils/Logger.hpp"

namespace webserv {

Socket::Socket() : type(kAny), fd(-1), addr_len(sizeof(addr))
{ ft_memset(&addr, 0, addr_len); }

Socket::~Socket()
{
	if (close(fd) < 0) {
		LOG_ERROR("Failed to close file descriptor");
		LOG_DEBUG("fd=" << fd);
	}
}

Socket::Socket(enum SocketType t) : type(t), fd(-1), addr_len(sizeof(addr))
{ ft_memset(&addr, 0, addr_len); }

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