#ifndef UTILS_HPP
# define UTILS_HPP

# include <algorithm>
# include <stdexcept>
# include <string>

# include <stddef.h>

# include <netinet/in.h>

namespace	webserv
{

	void*		ft_memset(void* s, int c, size_t n);
	const char*	ft_inet_ntoa(struct in_addr in);
	void		ft_sleep(double seconds);
	bool		ft_charcmp_icase(const unsigned char c1,
									const unsigned char c2);
	bool		ft_strcmp_icase(const std::string& str1,
								const std::string& str2);
	std::string	ft_str_tolower(const std::string& str);
	std::string trim_left(const std::string &s);
	std::string trim_right(const std::string &s);
	std::string	trim(const std::string& str);

	struct location_compare {
		bool	operator()(const std::string& s1, const std::string& s2) const
		{
			return (std::lexicographical_compare(s2.begin(), s2.end(),
						s1.begin(), s1.end()));
		}
	};

	class	FatalErrorException: public std::runtime_error {
	public:
		FatalErrorException(const std::string& msg = "A fatal error occured")
				: std::runtime_error(msg) 
		{ }
	};

	class	LogicErrorException: public std::logic_error {
	public:
		LogicErrorException(const std::string& msg = "A logic error occured")
				: std::logic_error(msg)
		{ }
	};

}	// namespace webserv

#endif  // UTILS_HPP
