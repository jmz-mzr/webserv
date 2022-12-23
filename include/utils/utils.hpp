#ifndef UTILS_HPP
# define UTILS_HPP

# include <algorithm>
# include <string>

# include <netinet/in.h>
# include <arpa/inet.h>

# include <stddef.h>
# include <sys/types.h>

# include "core/Socket.hpp"

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
	std::string trimLeft(const std::string &s);
	std::string trimRight(const std::string &s);
	std::string	trim(const std::string& str);
	void		initSockAddr(sockaddr_in& addr);
	void		setSockAddr(sockaddr_in& addr, const in_addr_t ip,
													const uint16_t port);
	void		copySockAddr(sockaddr_in& addr, const sockaddr_in& src);
	uint64_t	getSockAddrId(sockaddr_in& addr);

	struct location_compare {
		bool	operator()(const std::string& s1, const std::string& s2) const
		{
			return (std::lexicographical_compare(s2.begin(), s2.end(),
						s1.begin(), s1.end()));
		}
	};

	struct strcmp_icase {
		bool	operator()(const std::string& s1, const std::string& s2) const
		{
			return (ft_strcmp_icase(s1, s2));
		}
	};

}	// namespace webserv

#endif  // UTILS_HPP
