#include <ctime>

namespace	webserv
{

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
