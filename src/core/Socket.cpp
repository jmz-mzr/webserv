#include "Socket.hpp"

namespace wsrv {

Socket::Socket()
{ return ; }

Socket::Socket(const Socket& src)
: fd(src.fd), addr(src.addr)
{ return ; }

Socket::~Socket()
{
	close(fd);
	return ;
}

Socket&
Socket::operator=(const Socket& rhs)
{
	fd = rhs.fd;
	addr = rhs.addr;
	return (*this);
}

int
Socket::get_fd()
{ return (fd); }

}	/* namespace wsrv */