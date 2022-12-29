# include <csignal>
# include <cstdlib>
# include <iostream>

# include "core/Webserv.hpp"
# include "utils/exceptions.hpp"
# include "utils/Logger.hpp"

# include "utils/utils.hpp"

static void	handleSigInt(int signum)
{
	(void)signum;
	std::cout << "\r\033[2K";
	webserv::Webserv::receivedSigInt = 1;
	LOG_INFO("SIGINT received");
}

int	main(int argc, char** argv) try
{
	webserv::Webserv	webserv;

	webserv.init(argc, argv);
	// From then on the server should never die: we must try-catch
	// everything in "run()" so it never comes back here
	std::signal(SIGINT, handleSigInt);
	webserv.run();
	return (EXIT_SUCCESS);
} catch (const std::exception& e) {
	return (EXIT_FAILURE);
}