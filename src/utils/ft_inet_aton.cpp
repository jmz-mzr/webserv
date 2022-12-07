#include <algorithm>
#include <cstdlib>
#include <limits>
#include <string>
#include <sstream>

       #include <sys/socket.h>

#include <arpa/inet.h>
#include <netinet/in.h>

namespace webserv {

int	ft_inet_aton(const char *cp, struct in_addr *inp)
{
	char*			endp = NULL;
	std::string 	byte;
	std::string		str(cp);
	unsigned long	parts[4] = {0};
	in_addr_t		val = 0;

	std::ptrdiff_t	dotCount = std::count(str.begin(), str.end(), '.');
	
	if (dotCount > 3)
		return (0);
	std::istringstream input(str);
	for (int i = 0; i <= dotCount; i++) {
		std::getline(input, byte, '.');
		unsigned long l = strtoul(byte.c_str(), &endp, 0);
		if ((l == std::numeric_limits<unsigned long>::max())
				|| (byte.c_str() == endp))
			return (0);
		val = static_cast<in_addr_t>(l);
		parts[i] = val;
	}

	switch (dotCount) {
	case 0: break;
	case 1:
		if ((val > 0xffffff) || (parts[0] > 0xff))
			return (0);
		val |= (parts[0] << 24);
		break;
	case 2:
		if ((val > 0xffff) || (parts[0] > 0xff) || (parts[1] > 0xff))
			return (0);
		val |= ((parts[0] << 24) | (parts[1] << 16));
		break;
	case 3:
		if ((val > 0xff) || (parts[0] > 0xff)
				|| (parts[1] > 0xff) || (parts[2] > 0xff))
			return (0);
		val |= ((parts[0] << 24) | (parts[1] << 16) | (parts[2] << 8));
		break;
	}

	if (inp != NULL)
		inp->s_addr = htonl(val);
	return (1);
}

}	// namespace webserv