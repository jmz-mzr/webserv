#include "core/ConnectSocket.hpp"

#include <cstring>
#include <cerrno>

#include "utils/Logger.hpp"

namespace	webserv
{

	/**************************************************************************/
	/*                       CONSTRUCTORS / DESTRUCTORS                       */
	/**************************************************************************/

	ConnectSocket::ConnectSocket(const std::string& ipAddr, uint16_t port):
												Socket(kConnect, ipAddr, port)
	{
		_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (_fd < 0) {
			LOG_DEBUG("socket() error: " << strerror(errno));
			throw FatalErrorException("Fatal error on socket() call");
		}
		_addr.sin_family = AF_INET;
		_addr.sin_port = htons(_port);
		_addr.sin_addr.s_addr = inet_addr(_ipAddr.c_str());
		if (_addr.sin_addr.s_addr == INADDR_NONE) {
			closeFd();
			LOG_DEBUG("inet_addr() error: invalid IP address");
			throw FatalErrorException("Fatal error on inet_addr() call");
		}
		if (connect(_fd, reinterpret_cast<struct sockaddr*>(&_addr),
					_addrLen) < 0) {
			closeFd();
			LOG_DEBUG("connect() error: " << strerror(errno));
			throw FatalErrorException("Fatal error on connect() call");
		}
		LOG_INFO("New connected socket");
		LOG_DEBUG("fd=" << _fd << " ; addr=" << _ipAddr << " ; port=" << _port);
	}

}	// namespace webserv
