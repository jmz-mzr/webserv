#include <netinet/in.h>		// struct in_addr

#include <sstream>
#include <string>

namespace	webserv
{

	const char*	ft_inet_ntoa(struct in_addr in)
	{
		std::ostringstream		stream;
		unsigned char*			bytes = reinterpret_cast<unsigned char*>(&in);
		static std::string		str;

		stream << static_cast<int>(bytes[0])
			<< "." << static_cast<int>(bytes[1])
			<< "." << static_cast<int>(bytes[2])
			<< "." << static_cast<int>(bytes[3]);
		str = stream.str();
		return (str.c_str());
	}

}	// namespace webserv
