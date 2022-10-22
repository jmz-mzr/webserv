#include "core/ListenSock.hpp"

#include <cstdlib>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

namespace webserv {

ListenSock::ListenSock(const char* ip_addr, uint16_t port)
{
	fd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, IPPROTO_TCP);
	if (fd < 0)
		exit(EXIT_FAILURE);

	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr(ip_addr);
	addr.sin_port = htons(port);
	if ((bind(fd, reinterpret_cast<struct sockaddr*>(&addr), kAddrLen)) < 0)
		exit(EXIT_FAILURE);

	if (listen(fd, 10) < 0)
		exit(EXIT_FAILURE);
}

}	// namespace webserv