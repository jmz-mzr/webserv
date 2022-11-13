#include <cctype>
#include <string>
#include <algorithm>

namespace	webserv
{

	static bool ft_charcmp_icase(const unsigned char c1, const unsigned char c2)
	{
		return (std::tolower(c1) == std::tolower(c2));
	}

	bool	ft_strcmp_icase(const std::string& str1, const std::string& str2)
	{
		if (str1.size() == str2.size()) {
			return (std::equal(str1.begin(), str1.end(), str2.begin(),
						&ft_charcmp_icase));
		}
		return (false);
	}

}	// namespace webserv
