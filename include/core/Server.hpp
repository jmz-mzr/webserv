#ifndef SERVER_HPP
#define SERVER_HPP

#include <map>

#include <sys/epoll.h>

#include "core/Socket.hpp"
#include "core/ListenSock.hpp"

namespace webserv {

class Server
{

private:
	Server();
	Server(const Server& src);

	Server&
	operator=(const Server& rhs);

	void
	epollMod(int op, int events, Socket *socket);

public:
	Server(const char* ip_addr, uint16_t port);
	~Server();


private:
	ListenSock				listener;
	std::map<int, Socket>	clients;
	int						epoll_fd;
	static const int		kMaxEvent = 65536;
	struct epoll_event      events[Server::kMaxEvent];

};

}	// namespace webserv

#endif	// SERVER_HPP
