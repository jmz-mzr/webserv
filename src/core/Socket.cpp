#include "core/Socket.hpp"
#include "utils/Logger.hpp"

#include <unistd.h>

namespace	webserv
{

	/**************************************************************************/
	/*                       CONSTRUCTORS / DESTRUCTORS                       */
	/**************************************************************************/

	Socket::Socket(const Type t) :
			_type(t)
			, _fd(-1)
			, _ipAddr("")
			, _port(0)
			, _addrLen(sizeof(_addr))
	{
		ft_memset(&_addr, 0, _addrLen);
	}

	Socket::Socket(const Type t, const std::string& ipAddr, const uint16_t port)
			: _type(t)
			, _ipAddr(ipAddr)
			, _port(port)
			, _addrLen(sizeof(_addr))
	{
		ft_memset(&_addr, 0, _addrLen);
	}

	Socket::Socket(const Socket& src) :
			_type(src._type)
			, _fd(src._fd)
			, _ipAddr(src._ipAddr)
			, _port(src._port)
			, _addr(src._addr)
			, _addrLen(src._addrLen)
	{
		LOG_INFO("Socket copied");
		LOG_DEBUG("fd=" << _fd << " ; " 
				<< "addr=" << _ipAddr << " ; "
				<< "port=" << _port);
	}

	Socket::~Socket()
	{
		if (_fd >= 0)
		{
			if (close(_fd) < 0) {
				LOG_WARN("Bad close() on fd=" << _fd);
			} else {
				LOG_INFO ("close(" << _fd << ")");
			}
		}
	}

	/**************************************************************************/
	/*                            MEMBER FUNCTIONS                            */
	/**************************************************************************/

}	// namespace webserv
