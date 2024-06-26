#ifndef LISTENSOCKET_HPP
# define LISTENSOCKET_HPP

# include <netinet/in.h>	// sockaddr_in

# include "core/Socket.hpp"

namespace	webserv
{
	class	ListenSocket: public Socket {
	public:
		ListenSocket(const sockaddr_in& address);
		ListenSocket(const ListenSocket& src): Socket(src) { }
		virtual ~ListenSocket() { }
	private:
		ListenSocket();

		ListenSocket&	operator=(const ListenSocket& rhs);

		static const int	_kListenBacklog = SOMAXCONN;
	};

}	// namespace webserv

#endif	// LISTENSOCKET_HPP
