#include "utils/utils.hpp"

#include <cstdlib>
#include <ctime>

namespace	webserv
{

	std::string	createRandomFilename(const std::string& path,
										const std::string& prefix)
	{
		static bool			srandInit = false;
		static const char	alphaNum[] = "0123456789"
										"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
										"abcdefghijklmnopqrstuvwxyz";
		static size_t		len = sizeof(alphaNum) - 1;
		static int			suffix = 0;
		std::string			tmpFilename;

		if (!srandInit) {
			std::srand(time(0));
			srandInit = true;
		}
		if (!path.empty()) {
			tmpFilename = path;
			if (*path.rbegin() != '/')
				tmpFilename += '/';
		} else
			tmpFilename = "/tmp/";
		tmpFilename += prefix;
		for (int i = 0; i < 10; ++i)
			tmpFilename += alphaNum[std::rand() % len];
		tmpFilename += to_string(std::abs(suffix++));
		return (tmpFilename);
	}

}	// namespace webserv
