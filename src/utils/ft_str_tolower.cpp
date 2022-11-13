#include <string>
#include <algorithm>
#include <cctype>

namespace	webserv
{

	// Indirect call to std::tolower in order to avoid compiler error
	static int	ft_tolower(int c)
	{
		return (std::tolower(c));
	}

	std::string	ft_str_tolower(const std::string& str)
	{
		std::string		strToLower(str);

		std::transform(strToLower.begin(), strToLower.end(),
				strToLower.begin(), &ft_tolower);
		return (strToLower);
	}

}	// namespace webserv
