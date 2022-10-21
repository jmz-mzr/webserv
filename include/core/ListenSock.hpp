#ifndef LISTENSOCK_HPP
#define LISTENSOCK_HPP

#include "Socket.hpp"

namespace wsrv {

class ListenSock : public Socket
{

public:
	ListenSock(const char* ip_addr, uint16_t port);
	ListenSock(const ListenSock& src);

public:
	enum { type = ST_LIST };

};	/* class ListenSock */

}	/* namespace wsrv */

#endif	/* LISTENSOCK_HPP */