#include <cctype>
#include <string>
#include <algorithm>
#include <vector>

namespace	webserv
{

	bool	ft_charcmp_icase(const unsigned char c1, const unsigned char c2)
	{
		return (std::tolower(c1) == std::tolower(c2));
	}

	bool	ft_charcmp_icase2(const unsigned char c1, const unsigned char c2)
	{
		return (std::tolower(c1) < std::tolower(c2));
	}

	bool	ft_strcmp_icase(const std::string& str1, const std::string& str2)
	{
		if (str1.size() == str2.size()) {
			return (std::equal(str1.begin(), str1.end(), str2.begin(),
						&ft_charcmp_icase));
		}
		return (false);
	}

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

	std::vector<std::string>	ft_string_split(const std::string& str,
												const std::string& delim)
	{
		std::string					strPart;
		std::vector<std::string>	strVector;
		size_t						pos = str.find(delim);

		while (pos != std::string::npos)
		{
			strPart = str.substr(0, pos);
			strVector.push_back(strPart);
			pos = str.find(delim, pos + 1);
		}
		return (strVector);
	}

	std::string	ft_string_remove(std::string str, const char token)
	{
		std::string::iterator	c = str.begin();

		while (c != str.end()) {
			if (*c == token)
				c = str.erase(c);
			else
				++c;
		}
		return (str);
	}

}	// namespace webserv
