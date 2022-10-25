#include "core/ConnectSock.hpp"

#include <cstdlib>

#include "utils/Logger.hpp"

namespace webserv {

ConnectSock::ConnectSock() : Socket(kConnection)
{ }

ConnectSock::ConnectSock(int listener_fd) : Socket(kConnection)
{
    fd = accept(listener_fd, (struct sockaddr *)&addr, (socklen_t *)&addr_len);
    if (fd < 0)
    {
        LOG_ERROR("accept() failed");
        exit(EXIT_FAILURE);
    }
    LOG_INFO("New connection socket");
    LOG_DEBUG("fd=" << fd << " ; "
                << "addr=" << addr.sin_addr.s_addr << " ; "
                << "port=" << addr.sin_port);
}

ConnectSock::ConnectSock(const ConnectSock& src) : Socket(kConnection)
{ *this = src; }

ConnectSock::~ConnectSock() { }


}	// namespace webserv