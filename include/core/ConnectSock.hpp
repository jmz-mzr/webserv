#ifndef CONNECTSOCK_HPP
#define CONNECTSOCK_HPP

#include "core/Socket.hpp"

namespace webserv {

class ConnectSock : public Socket
{

public:
    ConnectSock();
    ConnectSock(int filedes, struct sockaddr_in address, socklen_t address_len);
    ConnectSock(const ConnectSock& src);
    ~ConnectSock();

    static const socklen_t	kAddrLen = sizeof(addr);


};

}	// namespace webserv

#endif	// CONNECTSOCK_HPP