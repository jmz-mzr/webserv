#include <libgen.h>		// dirname
#include <sys/stat.h>	// stat, S_ISDIR

#include <cctype>		// isprint
#include <cerrno>		// errno
#include <cstdio>
#include <cstdlib>		// abs, srand, rand
#include <cstring>		// memset
#include <ctime>		// time

#include <fstream>
#include <ios>
#include <iostream>
#include <string>

#include "utils/utils.hpp"

namespace	webserv
{

	long	getFileSize(const std::string& filename)
	{
		struct stat		stat_buf;
		int 			rc = stat(filename.c_str(), &stat_buf);

		return ( (rc == 0) ? stat_buf.st_size : -1 );
	}

	long	fdGetFileSize(int fd)
	{
		struct stat		stat_buf;
		int 			rc = fstat(fd, &stat_buf);

		return ( (rc == 0) ? stat_buf.st_size : -1 );
	}

	std::string	getFileExtension(const std::string& path)
	{
		const char*	file = path.c_str();
		int			i = static_cast<int>(path.size()) - 1;
		int			ext = 0;

		if (i <= 1 || !std::isprint(file[i])
				|| file[i] == '/' || file[i] == '.')
			return ("");
		while (--i > 0) {
			if (!std::isprint(file[i]) || file[i] == '/')
				return ("");
			if (file[i] == '.') {
				for (int j = i - 1; j >= 0; --j) {
					if (!std::isprint(file[j]) || file[j] == '/')
						return ("");
					if (std::isprint(file[j]) && file[j] != '.') {
						ext = i + 1;
						break ;
					}
				}
				break ;
			}
		}
		if (ext != 0)
			return (&file[ext]);
		return ("");
	}

	static std::string	_assertUniqueness(const std::string& path,
											const std::string& prefix,
											const std::string& tmpFilename)
	{
		struct stat		fileInfos;
		static int		fatalError = 0;
		std::string		directory = tmpFilename;

		errno = 0;
		std::memset(&fileInfos, 0, sizeof(fileInfos));
		if (stat(tmpFilename.c_str(), &fileInfos) == 0) {
			fatalError = 0;
			return (createRandomFilename(path,
						prefix + to_string(std::rand() % 10)));
		}
		std::memset(&fileInfos, 0, sizeof(fileInfos));
		if (errno != ENOENT || stat(dirname(const_cast<char*>
									(directory.c_str())), &fileInfos) < 0
				|| !S_ISDIR(fileInfos.st_mode)) {
			if (fatalError > 1)
				return ("");
			++fatalError;
			if (fatalError == 1)
				return (createRandomFilename("/tmp/", ""));
			else
				return (createRandomFilename("/var/tmp/", ""));
		}
		fatalError = 0;
		return (tmpFilename);
	}

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
			std::srand(std::time(0));
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
		return (_assertUniqueness(path, prefix, tmpFilename));
	}

	int moveFile(const char* from, const char* to)
	{
		std::ifstream in(from, std::ios::in | std::ios::binary);
		std::ofstream out(to, std::ios::out | std::ios::binary);

		if (in.fail() || out.fail())
			return (-1);
		out << in.rdbuf();
		return (std::remove(from));
	}

}	// namespace webserv
