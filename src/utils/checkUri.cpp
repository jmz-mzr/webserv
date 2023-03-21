#include <cctype>	// isalnum, isalpha, isdigit

#include <string>

namespace	webserv
{

	bool	isToken(const char c)
	{
		if (std::isalnum(c) || c == '!' || c == '#' || c == '$' || c == '%'
				|| c == '&' || c == '\'' || c == '*' || c == '+' || c == '-'
				|| c == '.' || c == '^' || c == '_' || c == '`' || c == '|'
				|| c == '~')
			return (true);
		return (false);
	}

	bool	isUriUnreserved(const char c)
	{
		if (std::isalnum(c) || c == '-' || c == '.' || c == '_' || c == '~')
			return (true);
		return (false);
	}

	bool	isUriPctEncoded(const char* firstOfThreeChar)
	{
		const char*					c = firstOfThreeChar;
		static const std::string	hex = "0123456789ABCDEFabcdef";

		if (c && *c == '%'
				&& *(c + 1) && hex.find(*(c + 1)) != std::string::npos
				&& *(c + 2) && hex.find(*(c + 2)) != std::string::npos)
			return (true);
		return (false);
	}

	bool	isUriSubDelim(const char c)
	{
		static const std::string	subDelim = "!$&\'()*+,;=";

		if (subDelim.find(c) != std::string::npos)
			return (true);
		return (false);
	}

	bool	isUriPChar(const char* c)
	{
		if (!c || !*c)
			return (false);
		if (isUriUnreserved(*c) || isUriPctEncoded(c) || isUriSubDelim(*c)
				|| *c == ':' || *c == '@')
			return (true);
		return (false);
	}

	size_t	checkUriScheme(const char* str, const std::string& delim)
	{
		size_t	i = 1;

		if (!str || !std::isalpha(*str))
			return (std::string::npos);
		while (str[i]) {
			if (std::isalnum(str[i]) || str[i] == '+' || str[i] == '-'
					|| str[i] == '.')
				++i;
			else if (delim.find(str[i]) != std::string::npos)
				return (i);
			else
				return (std::string::npos);
		}
		return (std::string::npos);
	}

	size_t	checkUriIpLiteral(const char* str, const std::string& delim)
	{
		size_t	i = 1;

		if (*str != '[')
			return (std::string::npos);
		while (str[i] != ']') {
			if (std::isalnum(str[i]) || isUriUnreserved(str[i])
					|| isUriSubDelim(str[i]) || str[i] == ':')
				++i;
			else if (str[i] != ']')
				return (std::string::npos);
		}
		++i;
		if (str[i] && delim.find(str[i]) == std::string::npos)
			return (std::string::npos);
		return (i);
	}

	size_t	checkUriHost(const char* str, const std::string& delim)
	{
		size_t	i = 0;

		if (!str)
			return (std::string::npos);
		if (*str == '[')
			return (checkUriIpLiteral(str, delim));
		while (str[i]) {
			if (std::isalnum(str[i]) || str[i] == '.' || str[i] == '-')
				++i;
			else if (delim.find(str[i]) != std::string::npos)
				break ;
			else
				return (std::string::npos);
			if (str[i] == '.' && str[i - 1] == '.')
				return (std::string::npos);
		}
		if (i == 1 && str[0] == '.')
			return (std::string::npos);
		return (i);
	}

	size_t	checkUriPort(const char* str, const std::string& delim)
	{
		size_t	i = 0;

		if (!str)
			return (std::string::npos);
		while (str[i]) {
			if (std::isdigit(str[i]))
				++i;
			else if (delim.find(str[i]) != std::string::npos)
				break ;
			else
				return (std::string::npos);
		}
		return (i);
	}

	size_t	checkUriAuthority(const char* str, const std::string& delim)
	{
		size_t	i;
		size_t	j = 0;

		if (!str)
			return (std::string::npos);
		if (str[0] != '/' || str[1] != '/')
			return (0);
		str += 2;
		i = checkUriHost(str, std::string(delim + ":"));
		if (i == std::string::npos)
			return (std::string::npos);
		if (str[i] == ':') {
			++i;
			j = checkUriPort(str + i, delim);
			if (j == std::string::npos)
				return (std::string::npos);
		}
		return (i + 2 + j);
	}

	size_t	checkUriPathAbEmpty(const char* str, const std::string& delim)
	{
		size_t	i = 0;

		if (!str || (*str && *str != '/'
					&& delim.find(str[i]) == std::string::npos))
			return (std::string::npos);
		while (str[i]) {
			if (str[i] == '/' || str[i] == ':' || str[i] == '@'
					|| isUriUnreserved(str[i]) || isUriSubDelim(str[i]))
				++i;
			else if (isUriPctEncoded(str + i))
				i += 3;
			else if (delim.find(str[i]) != std::string::npos)
				break ;
			else
				return (std::string::npos);
		}
		return (i);
	}

	size_t	checkUriPathAbs(const char* str, const std::string& delim)
	{
		size_t	i = 1;

		if (!str || *str != '/' || (str[i] && !isUriPChar(str + i)
				&& str[i] != '/' && delim.find(str[i]) == std::string::npos))
			return (std::string::npos);
		while (str[i]) {
			if (str[i] == '/' || str[i] == ':' || str[i] == '@'
					|| isUriUnreserved(str[i]) || isUriSubDelim(str[i]))
				++i;
			else if (isUriPctEncoded(str + i))
				i += 3;
			else if (delim.find(str[i]) != std::string::npos)
				break ;
			else
				return (std::string::npos);
		}
		return (i);
	}

	size_t	checkUriPathNoScheme(const char* str, const std::string& delim)
	{
		size_t	i = 1;

		if (!str || !isUriPChar(str) || *str == ':')
			return (std::string::npos);
		while (str[i] && isUriPChar(str + i) && str[i] != ':')
			++i;
		if (str[i] == ':' || (str[i] && str[i] != '/'
					&& delim.find(str[i]) == std::string::npos))
			return (std::string::npos);
		while (str[i]) {
			if (str[i] == '/' || str[i] == ':' || str[i] == '@'
					|| isUriUnreserved(str[i]) || isUriSubDelim(str[i]))
				++i;
			else if (isUriPctEncoded(str + i))
				i += 3;
			else if (delim.find(str[i]) != std::string::npos)
				break ;
			else
				return (std::string::npos);
		}
		return (i);
	}

	size_t	checkUriPathRootless(const char* str, const std::string& delim)
	{
		size_t	i = 1;

		if (!str || !isUriPChar(str))
			return (std::string::npos);
		while (str[i] && isUriPChar(str + i))
			++i;
		if (str[i] && str[i] != '/' && delim.find(str[i]) == std::string::npos)
			return (std::string::npos);
		while (str[i]) {
			if (str[i] == '/' || str[i] == ':' || str[i] == '@'
					|| isUriUnreserved(str[i]) || isUriSubDelim(str[i]))
				++i;
			else if (isUriPctEncoded(str + i))
				i += 3;
			else if (delim.find(str[i]) != std::string::npos)
				break ;
			else
				return (std::string::npos);
		}
		return (i);
	}

	size_t	checkUriPathEmpty(const char* str, const std::string& delim)
	{
		if (!str || (*str && delim.find(*str) == std::string::npos))
			return (std::string::npos);
		return (0);
	}

	static bool	assertUriPath(const char* str, size_t i)
	{
		if (i == std::string::npos)
			return (false);
		if (str[i] == '#') {
			if (i == 0 || (str[i - 1] != '?'
						&& std::string(str, i).find('/') == std::string::npos))
				return (false);
		}
		return (true);
	}

	size_t	checkUriPath(const char* str, bool hasAuthority,
							bool checkPathRootless, const std::string& delim)
	{
		size_t	i = 0;

		if (!str || (hasAuthority && *str
					&& delim.find(*str) == std::string::npos && *str != '/')
				|| (!hasAuthority && str[0] == '/' && str[1] == '/'))
			return (std::string::npos);
		i = checkUriPathAbEmpty(str, delim);
		if (i != std::string::npos)
			return (i);
		i = checkUriPathAbs(str, delim);
		if (i != std::string::npos)
			return (i);
		i = checkUriPathNoScheme(str, delim);
		if (i != std::string::npos)
			return (i);
		if (checkPathRootless)
			i = checkUriPathRootless(str, delim);
		if (i == std::string::npos)
			i = checkUriPathEmpty(str, delim);
		if (!assertUriPath(str, i))
			return (std::string::npos);
		return (i);
	}

	static size_t	checkUriQueryOrFragment(const char* str,
											const std::string& delim)
	{
		size_t	i = 0;

		if (!str)
			return (std::string::npos);
		while (str[i]) {
			if (str[i] == '/' || str[i] == '?' || str[i] == ':' || str[i] == '@'
					|| isUriUnreserved(str[i]) || isUriSubDelim(str[i]))
				++i;
			else if (isUriPctEncoded(str + i))
				i += 3;
			else if (delim.find(str[i]) != std::string::npos)
				break ;
			else
				return (std::string::npos);
		}
		return (i);
	}

	size_t	checkUriQuery(const char* str, const std::string& delim)
	{
		return (checkUriQueryOrFragment(str, delim));
	}

	size_t	checkUriFragment(const char* str, const std::string& delim)
	{
		return (checkUriQueryOrFragment(str, delim));
	}

}	// namespace webserv
