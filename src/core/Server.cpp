#include "core/Server.hpp"

#include <cstdlib>
#include <map>

#include <sys/epoll.h>
#include <unistd.h>

#include "core/ConnectSock.hpp"
#include "core/ListenSock.hpp"
#include "core/Socket.hpp"
#include "utils/Logger.hpp"

namespace webserv {

Server::Server() : listener(NULL, 0), clients() { }

Server::Server(const char* ip_addr, uint16_t port)
		: listener(ip_addr, port), clients()
{
	if ((epoll_fd = epoll_create1(0)) < 0)
	{
		LOG_ERROR("epoll instance creation failed");
		exit(EXIT_FAILURE);
	}
	LOG_INFO("New epoll instance");
	LOG_DEBUG("fd=" << epoll_fd);

	epollMod(EPOLL_CTL_ADD, EPOLLIN, &listener);
}

Server::Server(const Server& src)
		: listener(src.listener), clients(src.clients)
{ }

Server::~Server(void)
{
	if (close(epoll_fd) < 0)
		LOG_WARN("epoll instance didn't close");
}

Server&	Server::operator=(const Server& rhs)
{
	listener = rhs.listener;
	clients = rhs.clients;
	epoll_fd = rhs.epoll_fd;
	return *this;
}

void	Server::epollMod(int op, int events, Socket *socket)
{
	struct epoll_event	ev;

	ev.events = events;
	ev.data.ptr = socket;
	if (epoll_ctl(epoll_fd, op, socket->getFd(), &ev) < 1)
		LOG_WARN("epoll_ctl() failed");
}

int		Server::eventLoop()
{
	int     nevent;

	while (1)
	{
		if ((nevent = epoll_wait(epoll_fd, events, kMaxEvent, -1)) < 0) {
			LOG_ERROR("epoll_wait() failed");
			return EXIT_FAILURE;
		}
		for (int i = 0; i < nevent; i++) {
			Socket	*sock = reinterpret_cast<Socket *>(events[i].data.ptr);
			uint    ev = events[i].events;

			if (sock->type == kListen) {
				addClient();
				continue ;
			}
			else if (ev & (EPOLLERR | EPOLLHUP)) {
				rmClient(sock->getFd());
				continue ;
			}
			// if (ev & EPOLLIN) {
			// 	if (handle_recv(fd) < 0)
			// 		continue ;
			// }
			// if (ev & EPOLLOUT)
			// 	handle_send(fd);
		}
	}
}

void	Server::addClient(void)
{
	ConnectSock	client(listener.getFd());

	clients[client.getFd()] = client;
	LOG_INFO("New client accepted");
}

void	Server::rmClient(int fd)
{
	clients.erase(fd);
	LOG_INFO("Client deleted");
}

}	// namespace webserv