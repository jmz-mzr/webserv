#ifndef UTILS_HPP
# define UTILS_HPP

# include <netinet/in.h>	// in_addr_t, sockaddr_in, struct in_addr
# include <sys/stat.h>		// stat, S_ISDIR
# include <stddef.h>		// size_t
# include <stdint.h>		// uint16_t, uint64_t

# include <algorithm>		// lexicographical_compare
# include <utility>			// pair
# include <sstream>
# include <string>
# include <vector>

# include "core/Socket.hpp"

namespace	webserv
{

	const char*	ft_inet_ntoa(struct in_addr in);

	void	initSockAddr(sockaddr_in& addr);
	void	setSockAddr(sockaddr_in& addr, const in_addr_t ip,
							const uint16_t port);
	void		copySockAddr(sockaddr_in& addr, const sockaddr_in& src);
	uint64_t	getSockAddrId(sockaddr_in& addr);

	void	ft_sleep(double seconds);

	bool		isnotprint(int ch);

	bool		ft_charcmp_icase(const unsigned char c1,
									const unsigned char c2);
	bool		ft_charcmp_icase2(const unsigned char c1,
									const unsigned char c2);
	bool		ft_strcmp_icase(const std::string& str1,
								const std::string& str2);
	std::string	ft_str_tolower(const std::string& str);
	std::string	ft_str_toupper(const std::string& str);

	std::string	strHexDump(const std::string& str);

	std::vector<std::string>	ft_string_split(const std::string& str,
												const std::string& delim);
	std::string					ft_string_remove(const std::string& str,
													const char token);

	std::string trimLeft(const std::string& str);
	std::string trimRight(const std::string& str);
	std::string	trim(const std::string& str);

	long		getFileSize(const std::string& filename);
	long		fdGetFileSize(int fd);
	std::string	createRandomFilename(const std::string& path = "/tmp/",
										const std::string& prefix = "");
	std::string	getFileExtension(const std::string& path);

	bool	isUriPChar(const char* c);
	bool	isUriUnreserved(const char c);
	bool	isUriPctEncoded(const char* firstOfThreeChar);
	bool	isUriSubDelim(const char c);
	size_t	checkUriScheme(const char* str, const std::string& delim = ":");
	size_t	checkUriAuthority(const char* str,
								const std::string& delim = "/? ");
	size_t	checkUriHost(const char* str, const std::string& delim = ":/? ");
	size_t	checkUriIpLiteral(const char* str,
								const std::string& delim = ":/? ");
	size_t	checkUriPort(const char* str, const std::string& delim = "/? ");
	size_t	checkUriPath(const char* str, bool hasAuthority,
							bool checkPathRootless = true,
							const std::string& delim = "?# ");
	size_t	checkUriPathAbEmpty(const char* str,
								const std::string& delim = "?# ");
	size_t	checkUriPathAbs(const char* str, const std::string& delim = "?# ");
	size_t	checkUriPathNoScheme(const char* str,
									const std::string& delim = "?# ");
	size_t	checkUriPathRootless(const char* str,
									const std::string& delim = "?# ");
	size_t	checkUriPathEmpty(const char* str,
								const std::string& delim = "?# ");
	size_t	checkUriQuery(const char* str, const std::string& delim = "# ");
	size_t	checkUriFragment(const char* str, const std::string& delim = " ");

	struct dir_entry_compare {
		bool	operator()(const std::pair<std::string, struct stat>& p1,
							const std::pair<std::string, struct stat>& p2) const
		{
			if (S_ISDIR(p1.second.st_mode) && !S_ISDIR(p2.second.st_mode))
				return (true);
			if (!S_ISDIR(p1.second.st_mode) && S_ISDIR(p2.second.st_mode))
				return (false);
			return (std::lexicographical_compare(p1.first.begin(),
						p1.first.end(), p2.first.begin(), p2.first.end()));
		}
	};

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
			return (std::lexicographical_compare(s1.begin(), s1.end(),
						s2.begin(), s2.end(), &ft_charcmp_icase2));
		}
	};

	template <typename T>
	std::string to_string(T number)
	{
		std::ostringstream	numberString;

		numberString << number;
		return (numberString.str());
	}

}	// namespace webserv

#endif	// UTILS_HPP
