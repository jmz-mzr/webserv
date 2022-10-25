#include "core/ListenSock.hpp"

#include <cstdlib>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "utils/Logger.hpp"

namespace webserv {

ListenSock::ListenSock() : Socket(kListen) { }

ListenSock::ListenSock(const char* ip_addr, uint16_t port) : Socket(kListen)
{
	fd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, IPPROTO_TCP);
	if (fd < 0)
		exit(EXIT_FAILURE);

	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr(ip_addr);
	addr.sin_port = htons(port);
	if ((bind(fd, reinterpret_cast<struct sockaddr*>(&addr), addr_len)) < 0)
		exit(EXIT_FAILURE);

	if (listen(fd, 10) < 0)
		exit(EXIT_FAILURE);
	
	LOG_INFO("New listening socket");
	LOG_DEBUG("fd=" << fd << " ; "
            << "addr=" << addr.sin_addr.s_addr << " ; "
            << "port=" << addr.sin_port);
}

ListenSock::ListenSock(const ListenSock& src) : Socket(kListen)
{ *this = src; }

ListenSock::~ListenSock() { }

}	// namespace webserv