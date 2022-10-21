#include "Server.hpp"

namespace wsrv {

Server::Server()
: listener(NULL, 0), clients()
{
	return;
}

Server::Server(const char* ip_addr, uint16_t port)
: listener(ip_addr, port), clients()
{
	// create an epoll instance
    if ((epoll_fd = epoll_create1(0)) < 0)
        exit(EXIT_FAILURE);

    // add listening socket to epoll instance
    epoll_mod(EPOLL_CTL_ADD, EPOLLIN, &listener);
}

Server::Server(const Server& src)
: listener(src.listener), clients(src.clients)
{ return; }

Server::~Server(void)
{
	close(epoll_fd);
	return;
}

Server&
Server::operator=(const Server& rhs)
{
	listener = rhs.listener;
	clients = rhs.clients;
	epoll_fd = rhs.epoll_fd;
	return (*this);
}

void
Server::epoll_mod(int op, int events, Socket *socket)
{
	struct epoll_event	ev;

	ev.events = events;
	ev.data.ptr = socket;
	if (epoll_ctl(epoll_fd, op, socket->get_fd(), &ev) < 1)
		exit(EXIT_FAILURE);
}

}	/* namespace wsrv */