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
	for (std::map<int, ConnectSock *>::iterator it = clients.begin();
			it != clients.end();
			it++) {
		delete it->second;
	}
	if (close(epoll_fd) < 0)
		LOG_WARN("Bad close() on fd=" << epoll_fd);
	LOG_INFO("close(" << epoll_fd << ")");
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
	if (epoll_ctl(epoll_fd, op, socket->getFd(), &ev) < 0)
		LOG_WARN("epoll_ctl() failed");
}

int		Server::handle_event(Socket* socket, uint event)
{
	if (socket->type == kListen) {
		return addClient();
	} else if (event & (EPOLLERR | EPOLLHUP)) {
		rmClient(socket->getFd());
	}
	// if (ev & EPOLLIN) {
	// 	if (handle_recv(fd) < 0)
	// 		continue ;
	// }
	// if (ev & EPOLLOUT)
	// 	handle_send(fd);
	return (0);
}

int		Server::eventLoop()
{
	int     nevent;
	bool	stop = false;

	do {
		if ((nevent = epoll_wait(epoll_fd, events, kMaxEvent, -1)) < 0) {
			if (errno == EINTR)
				return EXIT_SUCCESS;
			LOG_ERROR("epoll_wait() failed");
			return EXIT_FAILURE;
		}
		for (int i = 0; i < nevent; i++) {
			stop = handle_event(reinterpret_cast<Socket *>(events[i].data.ptr),
							events[i].events);
			if (stop == true)
				break ;
		}
	} while (stop == false);
	return (EXIT_FAILURE);
}

int	Server::addClient(void)
{
	struct sockaddr_in	addr;
	socklen_t			addr_len;
	int					conn_fd;
	int					listen_fd = listener.getFd();

	while (true) {
		conn_fd = accept(listen_fd,
						(struct sockaddr *)&addr,
						(socklen_t *)&addr_len);
		if (conn_fd < 0) {
			if (errno != EWOULDBLOCK) {
				LOG_ERROR("accept() failed");
				return (-1);
			}
			break ;
		}	
		clients[conn_fd] = new ConnectSock(conn_fd, addr, addr_len);
		LOG_INFO("New client accepted");
	}
	LOG_INFO("No more pending connection");
	return (0);
}

void	Server::rmClient(int fd)
{
	std::map<int, ConnectSock *>::iterator it = clients.find(fd);
	delete it->second;
	clients.erase(fd);
	LOG_INFO("Client deleted");
}

const char*	Server::AcceptFailedException::what() const throw()
{
	return ("Accept() failed");
}

}	// namespace webserv