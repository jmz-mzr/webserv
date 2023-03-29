#include <cctype>		// isprint, tolower, toupper

#include <algorithm>	// equal, transform
#include <iostream>		// hex, uppercase
#include <sstream>
#include <string>
#include <vector>

namespace	webserv
{

	bool	isnotprint(int ch)
	{
		return (std::isprint(ch) == 0);
	}

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

	static int	ft_toupper(int c)
	{
		return (std::toupper(c));
	}

	std::string	ft_str_toupper(const std::string& str)
	{
		std::string		strToUpper(str);

		std::transform(strToUpper.begin(), strToUpper.end(),
				strToUpper.begin(), &ft_toupper);
		return (strToUpper);
	}

	std::string	strHexDump(const std::string& str)
	{
		std::string::const_iterator	c = str.begin();
		std::ostringstream			stream;

		stream << std::hex << std::uppercase;
		while (c != str.end()) {
			if (std::isprint(*c))
				stream << *c;
			else {
				if (*c < 16)
					stream << "\\x0";
				else
					stream << "\\x";
				stream << static_cast<int>(*c);
			}
			++c;
		}
		return (stream.str());
	}

	std::vector<std::string>	ft_string_split(const std::string& str,
												const std::string& delim)
	{
		std::string					strPart;
		std::vector<std::string>	strVector;
		size_t						pos = str.find(delim);

		if (pos == std::string::npos) {
			strVector.push_back(str);
		} else {
			while (pos != std::string::npos)
			{
				strPart = str.substr(0, pos);
				strVector.push_back(strPart);
				pos = str.find(delim, pos + 1);
			}
		}
		return (strVector);
	}

	std::string	ft_string_remove(const std::string& str, const char token)
	{
		std::string::const_iterator	c = str.begin();
		std::string					result;

		while (c != str.end()) {
			if (*c != token)
				result += *c;
			++c;
		}
		return (result);
	}

}	// namespace webserv
