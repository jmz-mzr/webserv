#ifndef LISTENSOCK_HPP
#define LISTENSOCK_HPP

#include "core/Socket.hpp"

namespace webserv {

class ListenSock : public Socket
{

public:
	ListenSock(const char* ip_addr, uint16_t port);
	ListenSock(const ListenSock& src);
	~ListenSock();

private:
	ListenSock();

	static const int	kListenBacklog = 128;


};

}	// namespace webserv

#endif /* LISTENSOCK_HPP */
