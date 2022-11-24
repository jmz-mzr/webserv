#ifndef UTILS_HPP
# define UTILS_HPP

# include <string>
# include <exception>
# include <algorithm>

# include <stddef.h>

# include <netinet/in.h>

namespace	webserv
{

	void*		ft_memset(void* s, int c, size_t n);
	const char*	ft_inet_ntoa(struct in_addr in);
	void		ft_sleep(double seconds);
	bool		ft_strcmp_icase(const std::string& str1,
								const std::string& str2);
	std::string	ft_str_tolower(const std::string& str);
	std::string	trim(const std::string& str);

	struct location_compare {
		bool	operator()(const std::string& s1, const std::string& s2) const
		{
			return (std::lexicographical_compare(s2.begin(), s2.end(),
						s1.begin(), s1.end()));
		}
	};

	class	FatalErrorException: public std::exception {
	public:
		FatalErrorException(const char* msg = "A fatal error occured"):
															_msg(msg) { }
		virtual const char*	what() const throw() { return (_msg); }
	private:
		const char*	_msg;
	};

	class	LogicErrorException: public std::exception {
	public:
		LogicErrorException(const char* msg = "A logic error occured"):
															_msg(msg) { }
		virtual const char*	what() const throw() { return (_msg); }
	private:
		const char*	_msg;
	};

}	// namespace webserv

#endif  // UTILS_HPP
