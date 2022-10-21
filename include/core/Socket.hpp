#ifndef SOCKET_HPP
#define SOCKET_HPP

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstdlib>

#include "Logger.hpp"

namespace wsrv {

class Socket
{

public:
	Socket();
	Socket(const Socket& src);
	virtual ~Socket();

	Socket&
	operator=(const Socket& rhs);

	int
	get_fd();

protected:
	int						fd;
	struct sockaddr_in		addr;
	static const socklen_t	addr_len = sizeof(addr);

public:
	enum { type = ST_ANY };


};	/* class Socket */

}	/* namespace wsrv */

#endif	/* SOCKET_HPP */