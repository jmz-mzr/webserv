#include <algorithm>
#include <cstdlib>
#include <climits>
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
	in_addr_t		parts[4] = {0};
	int 			i = 0;
	unsigned long	l;

	std::ptrdiff_t	dotCount = std::count(str.begin(), str.end(), '.');
	if (dotCount > 3)
		return (0);

	std::istringstream input(str);

	while (i <= dotCount) {
		std::getline(input, byte, '.');
		l = strtoul(byte.c_str(), &endp, 0);
		if ((l == ULONG_MAX) || (byte.c_str() == endp))
			return (0);
		parts[i] = static_cast<in_addr_t>(l);
		i++;
	}

	switch (dotCount) {
	case 0: break;
	case 1:
		if ((parts[i] > 0xffffff) || (parts[0] > 0xff))
			return (0);
		parts[i] |= (parts[0] << 24);
		break;
	case 2:
		if ((parts[i] > 0xffff) || (parts[0] > 0xff) || (parts[1] > 0xff))
			return (0);
		parts[i] |= ((parts[0] << 24) | (parts[1] << 16));
		break;
	case 3:
		if ((parts[i] > 0xff) || (parts[0] > 0xff)
				|| (parts[1] > 0xff) || (parts[2] > 0xff))
			return (0);
		parts[i] |= ((parts[0] << 24) | (parts[1] << 16) | (parts[2] << 8));
		break;
	}

	if (inp != NULL)
		inp->s_addr = htonl(parts[i]);
	return (1);
}

}	// namespace webserv