#ifndef ACCEPTSOCKET_HPP
# define ACCEPTSOCKET_HPP

# include "core/Socket.hpp"

namespace	webserv
{

	class	AcceptSocket: public Socket {
	public:
		AcceptSocket(const int fdListened);
		AcceptSocket(const AcceptSocket& src): Socket(src) { }
		virtual ~AcceptSocket() { }
	private:
		AcceptSocket();

		AcceptSocket&	operator=(const AcceptSocket& rhs);
	};

}	// namespace webserv

#endif	// ACCEPTSOCKET_HPP
