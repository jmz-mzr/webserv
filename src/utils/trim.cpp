#include <string>

namespace	webserv
{

static const std::string kWhitespaces = " \n\r\t\f\v";

std::string	trimLeft(const std::string& s)
{
	size_t start = s.find_first_not_of(kWhitespaces);
	return (start == std::string::npos) ? "" : s.substr(start);
}

std::string	trimRight(const std::string& s)
{
	size_t end = s.find_last_not_of(kWhitespaces);
	return (end == std::string::npos) ? "" : s.substr(0, end + 1);
}

std::string	trim(const std::string& s)
{
	return (trimRight(trimLeft(s)));
}

}	// namespace webserv
