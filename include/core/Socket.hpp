#ifndef SOCKET_HPP
# define SOCKET_HPP

# include <string>

# include <sys/types.h>		// getaddrinfo & co
# include <sys/socket.h>	// inet_addr, getaddrinfo & co
# include <netdb.h>			// getaddrinfo & co
# include <netinet/in.h>	// inet_addr
# include <arpa/inet.h>		// inet_addr

# include "utils/utils.hpp"

namespace	webserv
{

	class	Socket {
	public:
		enum Type {
			kListen,
			kAccept,
			kConnect
		};

		Socket(const Socket& src);
		virtual ~Socket() { };

		const Type&				getType() const { return (_type); }
		const int&				getFd() const { return (_fd); }
		const std::string&		getIpAddr() const { return (_ipAddr); }
		const uint16_t&			getPort() const { return (_port); }

		void					closeFd();

	protected:
		Socket(const Type t);
		Socket(const Type t, const std::string& ipAddr, const uint16_t port);

		const Type				_type;
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
