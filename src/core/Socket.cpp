#include "core/Socket.hpp"

#include <unistd.h>

namespace webserv {

Socket::Socket() { }

Socket::~Socket() { close(fd); }

Socket::Socket(const Socket& src) : fd(src.fd), addr(src.addr) { }

Socket&	Socket::operator=(const Socket& rhs)
{
	fd = rhs.fd;
	addr = rhs.addr;
	return *this;
}

const int&	Socket::getFd() const { return fd; }

}	// namespace webserv