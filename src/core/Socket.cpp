#include "core/Socket.hpp"

#include <cstring>

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
	{
		memset(&_addr, 0, _addrLen);
	}

	Socket::Socket(const Type t, const Address& address)
			: _type(t)
			, _fd(-1)
			, _addrLen(sizeof(_addr))
			, _port(address.port)
			, _id(address.id)
	{
		memset(&_addr, 0, _addrLen);
		_addr.sin_family = AF_INET;
		_addr.sin_port = htons(address.port);
		_addr.sin_addr.s_addr = address.ipAddr;
		if (_addr.sin_addr.s_addr == INADDR_NONE) {
			THROW_FATAL("inet_addr() error: invalid IP address");
		}
		if ((_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
			THROW_FATAL("socket() error: " << strerror(errno));
		}
		_ip = ft_inet_ntoa(_addr.sin_addr);
	}

	Socket::Socket(const Socket& src)
			: _type(src._type)
			, _fd(src._fd)
			, _addr(src._addr)
			, _addrLen(src._addrLen)
			, _ip(src._ip)
			, _port(src._port)
			, _id(src._id)
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
