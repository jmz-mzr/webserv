#ifndef UTILS_HPP
# define UTILS_HPP

# include <exception>

# include <stddef.h>

# include <netinet/in.h>

namespace	webserv
{

	void*		ft_memset(void* s, int c, size_t n);
	const char*	ft_inet_ntoa(struct in_addr in);
	void		ft_sleep(double seconds);

	class FatalErrorException: public std::exception {
	public:
		FatalErrorException(const char* msg = "A fatal error occured"):
															_msg(msg) { }
		virtual const char*	what() const throw() { return (_msg); }
	private:
		const char*	_msg;
	};

}	// namespace webserv

#endif  // UTILS_HPP
