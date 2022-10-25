#ifndef SOCKET_HPP
#define SOCKET_HPP

#include <netinet/in.h>
#include <sys/socket.h>

namespace webserv {

enum SocketType {
	kAny,
	kListen,
	kConnection
};

class Socket
{

public:
	Socket();
	Socket(const Socket& src);
	virtual ~Socket();

	Socket&	operator=(const Socket& rhs);

	const int&	getFd() const;

	const enum SocketType	type;


protected:
	Socket(enum SocketType type);

	int						fd;
	struct sockaddr_in		addr;
	socklen_t				addr_len;

};

}	// namespace webserv

#endif	// SOCKET_HPP