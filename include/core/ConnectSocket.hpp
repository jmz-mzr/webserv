#ifndef CONNECTSOCKET_HPP
# define CONNECTSOCKET_HPP

# include <cerrno>

# include "core/Socket.hpp"

namespace	webserv
{

	class	ConnectSocket: public Socket {
	public:
		ConnectSocket(const sockaddr_in& address);
		ConnectSocket(const ConnectSocket& src): Socket(src) { }
		virtual ~ConnectSocket() { }
	private:
		ConnectSocket();

		ConnectSocket&	operator=(const ConnectSocket& rhs);
	};

}	// namespace webserv

#endif	// CONNECTSOCKET_HPP
