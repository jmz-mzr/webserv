#include <cstdlib>

namespace webserv {

void	*ft_memset(void *s, int c, size_t n)
{
	char	*str;

	str = (char *)s;
	while (n--)
		*str++ = c;
	return (s);
}

}	// namespace webserv