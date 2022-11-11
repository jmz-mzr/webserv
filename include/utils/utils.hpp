#ifndef UTILS_HPP
# define UTILS_HPP

# include <stddef.h>

# include <netinet/in.h>

namespace	webserv
{

	void*		ft_memset(void* s, int c, size_t n);
	const char*	ft_inet_ntoa(struct in_addr in);
	void		ft_sleep(double seconds);

}	// namespace webserv

#endif  // UTILS_HPP
