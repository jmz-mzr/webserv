#include <string>
#include <algorithm>

#include "utils/utils.hpp"

namespace	webserv
{

	bool	ft_strcmp_icase(const std::string& str1, const std::string& str2)
	{
		if (str1.size() == str2.size()) {
			return (std::equal(str1.begin(), str1.end(), str2.begin(),
						&ft_charcmp_icase));
		}
		return (false);
	}

}	// namespace webserv
