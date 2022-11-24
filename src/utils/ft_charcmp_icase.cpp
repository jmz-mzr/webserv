#include <cctype>

namespace	webserv
{

	bool	ft_charcmp_icase(const unsigned char c1, const unsigned char c2)
	{
		return (std::tolower(c1) == std::tolower(c2));
	}

}	// namespace webserv
