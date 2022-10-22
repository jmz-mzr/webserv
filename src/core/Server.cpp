#include "core/Server.hpp"

#include <cstdlib>

#include <unistd.h>

namespace webserv {

Server::Server() : listener(NULL, 0), clients() { }

Server::Server(const char* ip_addr, uint16_t port)
		: listener(ip_addr, port), clients()
{
	if ((epoll_fd = epoll_create1(0)) < 0)
		exit(EXIT_FAILURE);

	epollMod(EPOLL_CTL_ADD, EPOLLIN, &listener);
}

Server::Server(const Server& src)
		: listener(src.listener), clients(src.clients)
{ }

Server::~Server(void) { close(epoll_fd); }

Server&
Server::operator=(const Server& rhs)
{
	listener = rhs.listener;
	clients = rhs.clients;
	epoll_fd = rhs.epoll_fd;
	return (*this);
}

void
Server::epollMod(int op, int events, Socket *socket)
{
	struct epoll_event	ev;

	ev.events = events;
	ev.data.ptr = socket;
	if (epoll_ctl(epoll_fd, op, socket->getFd(), &ev) < 1)
		exit(EXIT_FAILURE);
}

}	// namespace webserv