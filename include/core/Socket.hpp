#ifndef SOCKET_HPP
# define SOCKET_HPP

# include <string>

# include <sys/socket.h>
# include <netinet/in.h>
# include <arpa/inet.h>

# include "utils/utils.hpp"

namespace	webserv
{

	enum SocketType {
		kListen,
		kAccept,
		kConnect
	};

	class	Socket {
	public:
		Socket(const Socket& src);
		virtual ~Socket() { }

		const enum SocketType&	getType() const { return (_type); }
		const int&				getFd() const { return (_fd); }
		const std::string&		getIpAddr() const { return (_ipAddr); }
		const uint16_t&			getPort() const { return (_port); }

		void	closeFd();
	protected:
		Socket(enum SocketType t): _type(t), _fd(-1), _ipAddr(""),
									_port(0), _addrLen(sizeof(_addr))
											{ ft_memset(&_addr, 0, _addrLen); }
		Socket(enum SocketType t, const std::string& ipAddr, uint16_t port):
									_type(t), _ipAddr(ipAddr), _port(port),
									_addrLen(sizeof(_addr))
											{ ft_memset(&_addr, 0, _addrLen); }

		const enum SocketType	_type;
		int						_fd;
		std::string				_ipAddr;
		uint16_t				_port;
		struct sockaddr_in		_addr;
		socklen_t				_addrLen;
	private:
		Socket();
		Socket&	operator=(const Socket& rhs);
	};

}	// namespace webserv

#endif	// SOCKET_HPP
