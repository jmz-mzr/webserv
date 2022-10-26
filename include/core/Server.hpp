#ifndef SERVER_HPP
#define SERVER_HPP

#include <map>

#include <sys/epoll.h>

#include "core/ConnectSock.hpp"
#include "core/ListenSock.hpp"
#include "core/Socket.hpp"

namespace webserv {

class Server
{

public:
	Server(const char* ip_addr, uint16_t port);
	~Server();

	int		eventLoop();

	class AcceptFailedException: public std::exception
	{
		public:
			virtual char const*	what() const throw();
	};


private:
	Server();
	Server(const Server& src);

	Server&	operator=(const Server& rhs);

	int		handle_event(Socket* socket, uint event);
	void	epollMod(int op, int events, Socket *socket);
	int		addClient(void);
	void	rmClient(int fd);

	static const int			kMaxEvent = 65536;

	ListenSock						listener;
	std::map<int, ConnectSock *>	clients;
	int								epoll_fd;
	struct epoll_event				events[Server::kMaxEvent];

};

}	// namespace webserv

#endif /* SERVER_HPP */
