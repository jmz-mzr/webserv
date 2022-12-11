#include "core/Socket.hpp"

#include <unistd.h>

#include "utils/exceptions.hpp"
#include "utils/Logger.hpp"
#include "utils/utils.hpp"

namespace	webserv
{

	/**************************************************************************/
	/*                       CONSTRUCTORS / DESTRUCTORS                       */
	/**************************************************************************/

	Socket::Socket(const Type t)
			: _type(t)
			, _fd(-1)
			, _addrLen(sizeof(_addr))
	{ memset(&_addr, 0, _addrLen); }

	Socket::Socket(const Type t, const sockaddr_in& address)
			: _type(t)
			, _fd(-1)
			, _addrLen(sizeof(address))
	{
		memset(&_addr, 0, _addrLen);
		_addr.sin_addr.s_addr = address.sin_addr.s_addr;
		_addr.sin_port = address.sin_port;
		_addr.sin_family = address.sin_family;
		_port = ntohs(_addr.sin_port);
		_ip = ft_inet_ntoa(_addr.sin_addr);

		if (_addr.sin_addr.s_addr == INADDR_NONE) {
			LOG_DEBUG("ip=" << _ip << " port=" << _port);
			THROW_FATAL("inet_addr() error: invalid IP address");
		}
		if ((_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
			LOG_DEBUG("ip=" << _ip << " port=" << _port);
			THROW_FATAL("socket() error: " << strerror(errno));
		}
	}

	Socket::Socket(const Socket& src)
			: _type(src._type)
			, _fd(src._fd)
			, _addr(src._addr)
			, _addrLen(src._addrLen)
			, _ip(src._ip)
			, _port(src._port)
	{
		LOG_INFO("Socket copied");
		LOG_DEBUG("fd=" << _fd << " ; " 
				<< "addr=" << _ip << " ; "
				<< "port=" << _port);
	}

	/**************************************************************************/
	/*                            MEMBER FUNCTIONS                            */
	/**************************************************************************/

	void	Socket::closeFd()
	{
		if (_fd >= 0) {
			if (close(_fd) < 0) {
				LOG_WARN("Bad close() on fd=" << _fd);
			} else {
				LOG_INFO("close(" << _fd << ")");
				_fd = -1;
			}
		}
	}


}	// namespace webserv
