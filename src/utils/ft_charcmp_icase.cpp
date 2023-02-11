#include <cctype>

namespace	webserv
{

	bool	ft_charcmp_icase(const unsigned char c1, const unsigned char c2)
	{
		return (std::tolower(c1) == std::tolower(c2));
	}

	/**
	 * @brief Function satisfying requirements for Compare
	 * @link https://en.cppreference.com/w/cpp/named_req/Compare
	 */
	bool	ft_charcmp_icase2(const unsigned char c1, const unsigned char c2)
	{
		return (std::tolower(c1) < std::tolower(c2));
	}

}	// namespace webserv
