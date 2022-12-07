#ifndef UTILS_HPP
# define UTILS_HPP

# include <algorithm>
# include <string>

# include <netinet/in.h>
# include <arpa/inet.h>

# include <stddef.h>

namespace	webserv
{

	const char*	ft_inet_ntoa(struct in_addr in);
	int			ft_inet_aton(const char *cp, struct in_addr *inp);
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

	struct listen_compare {
		bool	operator()(const sockaddr_in& s1, const sockaddr_in& s2) const
		{
			return ((s1.sin_port < s2.sin_port)
					|| (s1.sin_addr.s_addr < s2.sin_addr.s_addr));
		}
	};


}	// namespace webserv

#endif  // UTILS_HPP
