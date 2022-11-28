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

/*	ConnectSocket::ConnectSocket(const std::string& ipAddr, uint16_t port):
												Socket(kConnect, ipAddr, port)
	{
		// TO DO: Make it work also with IPv6 address?

		struct addrinfo		addrHints;
		struct addrinfo*	addrList;
		const char*			portStr = STRINGIZE(port);
		int					error;

		ft_memset(&addrHints, 0, sizeof(struct addrinfo));
		addrHints.ai_family = AF_INET;
		addrHints.ai_socktype = SOCK_STREAM;
		addrHints.ai_protocol = IPPROTO_TCP;
		addrHints.ai_flags = AI_NUMERICSERV | AI_CANONNAME;
		error = getaddrinfo(ipAddr.c_str(), portStr, &addrHints, &addrList);
		if (error) {
			if (error == EAI_SYSTEM) {
				LOG_DEBUG("getaddrinfo() error: " << strerror(errno));
			} else
				LOG_DEBUG("getaddrinfo() error: " << gai_strerror(error));
			throw FatalErrorException("Fatal error on getaddrinfo() call");
		}
		_addr = *(reinterpret_cast<sockaddr_in*>(addrList->ai_addr));
		_addrLen = addrList->ai_addrlen;
		if ((_fd = socket(addrList->ai_family, addrList->ai_socktype,
						addrList->ai_protocol)) < 0) {
			freeaddrinfo(addrList);
			LOG_DEBUG("socket() error: " << strerror(errno));
			throw FatalErrorException("Fatal error on socket() call");
		}
		if (connect(_fd, addrList->ai_addr, addrList->ai_addrlen) < 0) {
			closeFd();
			freeaddrinfo(addrList);
			LOG_DEBUG("connect() error: " << strerror(errno));
			throw FatalErrorException("Fatal error on connect() call");
		}
		freeaddrinfo(addrList);
		LOG_INFO("New connected socket");
		LOG_DEBUG("fd=" << _fd << " ; addr=" << _ipAddr << " ; port=" << _port);
	}*/

}	// namespace webserv
