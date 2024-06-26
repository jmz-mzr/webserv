#ifndef SOCKET_HPP
# define SOCKET_HPP

# include <netinet/in.h>	// sockaddr_in
# include <stdint.h>		// uint16_t
# include <sys/socket.h>	// socklen_t

# include <string>
# include <iostream>

namespace	webserv
{

	struct listen_compare {
	bool	operator()(const sockaddr_in& s1, const sockaddr_in& s2) const
	{
		return ((s1.sin_port < s2.sin_port) ||
				(s1.sin_addr.s_addr < s2.sin_addr.s_addr));
	}
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
		const sockaddr_in&		getAddr() const { return (_addr); }
		const std::string&		getIpAddr() const { return (_ip); }
		const uint16_t&			getPort() const { return (_port); }

		void					closeFd();

		friend std::ostream&	operator<<(std::ostream&, const Socket&);
	protected:
		explicit Socket(const Type t);
		Socket(const Type t, const sockaddr_in& addr);

		const Type				_type;
		int						_fd;
		sockaddr_in				_addr;
		socklen_t				_addrLen;
		std::string				_ip;
		uint16_t				_port;
	private:
		Socket();
		Socket&	operator=(const Socket& rhs);
	};

}	// namespace webserv

#endif	// SOCKET_HPP
