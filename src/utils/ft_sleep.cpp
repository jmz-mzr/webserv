#include <unistd.h>	// usleep

#include <ctime>	// clock, difftime, time

#include "utils/utils.hpp"

namespace	webserv
{

	void	ft_usleep(double seconds, size_t uIncrement)
	{
		std::time_t		start = std::time(0);

		if (uIncrement / 1000000 > seconds)
			uIncrement = seconds * 1000000;
		while (std::difftime(std::time(0), start) < seconds)
			usleep(uIncrement);
	}

	void	ft_sleep(double seconds)
	{
		std::clock_t		start = std::clock();
		volatile double 	someDbValue = 1.0;

		while ((static_cast<double>(std::clock() - start) / CLOCKS_PER_SEC)
				< seconds) {
			for (int consumeTime = 0; consumeTime < 1000000; ++consumeTime)
				someDbValue *= consumeTime * someDbValue * consumeTime;
		}
	}

}	// namespace webserv
