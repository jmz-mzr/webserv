#include "core/ConnectSock.hpp"

#include <cerrno>
#include <cstdlib>

#include "utils/Logger.hpp"

namespace webserv {

ConnectSock::ConnectSock() : Socket(kConnection)
{ }

ConnectSock::ConnectSock(int filedes,
                        struct sockaddr_in address,
                        socklen_t address_len)
        : Socket(kConnection, filedes, address, address_len)
{
    LOG_INFO("New connection socket");
    LOG_DEBUG("fd=" << fd << " ; "
                << "addr=" << addr.sin_addr.s_addr << " ; "
                << "port=" << addr.sin_port);
}

ConnectSock::ConnectSock(const ConnectSock& src) : Socket(kConnection)
{ *this = src; }

ConnectSock::~ConnectSock() { }

}	// namespace webserv