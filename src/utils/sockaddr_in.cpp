#include <cstring>

# include <netdb.h>			// getaddrinfo & co
# include <netinet/in.h>	// inet_addr
# include <arpa/inet.h>		// inet_addr

namespace webserv {

void		 initSockAddr(sockaddr_in& addr)
{ memset(&addr, 0, sizeof(addr)); }

void		 setSockAddr(sockaddr_in& addr, const in_addr_t ip,
													const uint16_t port)
{
	initSockAddr(addr);
	addr.sin_addr.s_addr = ip;
	addr.sin_port = htons(port);
	addr.sin_family = AF_INET;
}

void		copySockAddr(sockaddr_in& addr, const sockaddr_in& src)
{
	initSockAddr(addr);
	addr.sin_addr.s_addr = src.sin_addr.s_addr;
	addr.sin_port = src.sin_port;
	addr.sin_family = src.sin_family;
}

}	// namespace webserv
