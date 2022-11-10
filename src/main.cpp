#include <csignal>
#include <iostream>
#include <exception>

#include "core/Webserv.hpp"
#include "utils/Logger.hpp"

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

	(void)argc;
	webserv.loadConfig(argv[1]);
	// From then on the server should never die: we must try-catch
	// everything in "run()" so it never comes back here
	std::signal(SIGINT, handleSigInt);
	webserv.run();
	return (EXIT_SUCCESS);
}
catch (const std::exception& e) {
	LOG_ERROR("Fatal error: " << e.what());
	return (EXIT_FAILURE);
}
