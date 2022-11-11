#ifndef CONNECTSOCKET_HPP
# define CONNECTSOCKET_HPP

# include "core/Socket.hpp"

namespace	webserv
{

	class	ConnectSocket: public Socket {
	public:
		ConnectSocket(const std::string& ipAddr, uint16_t port);
		ConnectSocket(const ConnectSocket& src): Socket(src) { }
		virtual ~ConnectSocket() { }
	private:
		ConnectSocket();

		ConnectSocket&	operator=(const ConnectSocket& rhs);
	};

}	// namespace webserv

#endif	// CONNECTSOCKET_HPP
