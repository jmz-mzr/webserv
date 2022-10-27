#include "core/Server.hpp"

#include <cstdlib>
#include <map>

#include <fcntl.h>
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

void	Server::eventLoop()
{
	int     nevent;

	while (true) {
		if ((nevent = epoll_wait(epoll_fd, events, kMaxEvent, -1)) < 0) {
			if (errno == EINTR)
				throw Server::SignalException();
			LOG_ERROR("epoll_wait() failed");
			throw Server::FatalErrorException();
		}
		for (int i = 0; i < nevent; i++) {
			handle_event(reinterpret_cast<Socket *>(events[i].data.ptr),
								events[i].events);
		}
	}
}

void	Server::recv_data(int fd, uint& event)
{
	char	buffer[1024];
	int		bytes_recv;

	while ((bytes_recv = recv(fd, buffer, 1024 - 1, 0)) > 0) {
		buffer[bytes_recv] = '\0';
		LOG_INFO("recv: " << buffer);
	}
	if (bytes_recv == -1) {
		if (!((errno == EWOULDBLOCK) || (errno == EAGAIN))) {
			LOG_ERROR("recv() error");
			throw Server::FatalErrorException(); // fatal error exception
		} else if (errno == EINTR)
			throw Server::SignalException(); // signal exception
	}
	else if (bytes_recv == 0) {
		rmClient(fd);
		event &= ~EPOLLOUT;
	}
}

void	Server::handle_event(Socket* socket, uint event)
{
	int fd = socket->getFd();

	if (event & (EPOLLERR | EPOLLHUP)) {
		// Relancer le socket si listener ? throw fatal error ?
		rmClient(fd);
	} else if (socket->type == kListen) {
		addClient();
	} else {
		if (event & EPOLLIN) {
			recv_data(fd, event);
		}
		// TODO if (event & EPOLLOUT)
	}
}

void	Server::addClient(void)
{
	ConnectSock*		client;
	struct sockaddr_in	addr;
	socklen_t			addr_len;
	int					conn_fd;
	int					listen_fd = listener.getFd();

	while (true) {
		conn_fd = accept(listen_fd,
						reinterpret_cast<struct sockaddr *>(&addr),
						reinterpret_cast<socklen_t *>(&addr_len));
		if (conn_fd < 0) {
			if (errno != EWOULDBLOCK) {
				LOG_ERROR("accept() failed");
				throw Server::FatalErrorException();
			}
			break ;
		}
		fcntl(conn_fd, F_SETFL, fcntl(conn_fd, F_GETFL) | O_NONBLOCK);
		client = new ConnectSock(conn_fd, addr, addr_len);
		clients[conn_fd] = client;
		epollMod(EPOLL_CTL_ADD, EPOLLIN, client);
		LOG_INFO("New client accepted");
	}
	LOG_INFO("No more pending connection");
}

void	Server::rmClient(int fd)
{
	std::map<int, ConnectSock *>::iterator it = clients.find(fd);
	epollMod(EPOLL_CTL_DEL, 0, it->second);
	delete it->second;
	clients.erase(fd);
	LOG_INFO("Client deleted");
}

const char*	Server::SignalException::what() const throw()
{
	return ("A signal has been received");
}

const char*	Server::FatalErrorException::what() const throw()
{
	return ("A fatal error occured");
}

}	// namespace webserv