#include "ListenSock.hpp"

namespace wsrv {

ListenSock::ListenSock(const char* ip_addr, uint16_t port)
{
	// TCP non-blocking socket creation
	fd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, IPPROTO_TCP);
	if (fd < 0)
		exit(EXIT_FAILURE);

	// Binding socket to given IP and port
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr(ip_addr);
	addr.sin_port = htons(port);
	if ((bind(fd, (const struct sockaddr*)&addr, addr_len)) < 0)
		exit(EXIT_FAILURE);

	// listen for incoming connection
	if (listen(fd, 10) < 0)
		exit(EXIT_FAILURE);

	return;
}

ListenSock::ListenSock(const ListenSock& src)
: Socket(src)
{ return ; }

}	/* namespace wsrv */