#ifndef LISTENSOCKET_HPP
# define LISTENSOCKET_HPP

# include <cerrno>

# include "core/Socket.hpp"

namespace	webserv
{

	class	ListenSocket: public Socket {
	public:
		ListenSocket(const std::string& ipAddr, uint16_t port);
		ListenSocket(const ListenSocket& src): Socket(src) { }
		virtual ~ListenSocket() { }
	private:
		ListenSocket();

		ListenSocket&	operator=(const ListenSocket& rhs);

		static const int	_kListenBacklog = SOMAXCONN;
	};

}	// namespace webserv

#endif	// LISTENSOCKET_HPP
