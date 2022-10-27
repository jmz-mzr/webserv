#ifndef SERVER_HPP
#define SERVER_HPP

#include <exception>
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

	void	eventLoop();

	class SignalException: public std::exception
	{
		public:
			virtual const char*	what() const throw();
	};

	class FatalErrorException: public std::exception
	{
		public:
			virtual const char*	what() const throw();
	};


private:
	Server();
	Server(const Server& src);

	Server&	operator=(const Server& rhs);

	void	handle_event(Socket* socket, uint event);
	void	epollMod(int op, int events, Socket *socket);
	void	addClient(void);
	void	rmClient(int fd);
	void	recv_data(int fd, uint& event);

	static const int			kMaxEvent = 65536;

	ListenSock						listener;
	std::map<int, ConnectSock *>	clients;
	int								epoll_fd;
	struct epoll_event				events[Server::kMaxEvent];

};

}	// namespace webserv

#endif /* SERVER_HPP */
