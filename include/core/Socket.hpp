#ifndef SOCKET_HPP
# define SOCKET_HPP

# include <string>

# include <sys/types.h>		// getaddrinfo & co
# include <sys/socket.h>	// inet_addr, getaddrinfo & co
# include <netdb.h>			// getaddrinfo & co
# include <netinet/in.h>	// inet_addr
# include <arpa/inet.h>		// inet_addr

# include "utils/Logger.hpp"
# include "utils/utils.hpp"

namespace	webserv
{

	struct Address {
		Address(in_addr_t ip, uint16_t p)
				: ipAddr(ip)
				, port(p)
				, id(ntohl(ip) | (uint64_t(p) << 32))
		{
			LOG_INFO("New address instance");
			LOG_DEBUG(
				"ip=" << ft_inet_ntoa(*reinterpret_cast<in_addr*>(&ipAddr))
				<< " port=" << port);
		}

		friend bool	operator<(const Address& lhs, const Address& rhs)
		{ return (lhs.id < rhs.id); }

		in_addr_t		ipAddr;
		uint16_t		port;
		uint64_t		id;

	};

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
		const std::string&		getIpAddr() const { return (_ip); }
		const uint16_t&			getPort() const { return (_port); }
		const uint64_t&			getId() const { return (_id); }

		void					closeFd();

	protected:
		Socket(const Type t);
		Socket(const Type t, const Address& addr);

		const Type				_type;
		int						_fd;
		struct sockaddr_in		_addr;
		socklen_t				_addrLen;
		std::string				_ip;
		uint16_t				_port;
		uint64_t				_id;
	private:
		Socket();
		Socket&	operator=(const Socket& rhs);
	};

}	// namespace webserv

#endif	// SOCKET_HPP
