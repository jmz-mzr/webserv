#ifndef SOCKET_HPP
#define SOCKET_HPP

#include <netinet/in.h>
#include <sys/socket.h>

namespace webserv {

enum SocketType {
	kAny = 0x01,
	kListen = 0x10,
	kConnection = 0x11
};

class Socket
{

public:
	Socket();
	Socket(const Socket& src);
	virtual ~Socket();

	Socket&	operator=(const Socket& rhs);

	const int&	getFd() const;

protected:
	int						fd;
	struct sockaddr_in		addr;
	static const socklen_t	kAddrLen = sizeof(addr);

public:
	enum { type = kAny };

};

}	// namespace webserv

#endif	// SOCKET_HPP