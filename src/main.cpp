#include <csignal>		// signal, SIGINT
#include <cstdlib>		// EXIT_SUCCESS/FAILURE

#include <exception>
#include <iostream>		// cout

#include "core/Webserv.hpp"
#include "utils/log.hpp"
#include "utils/utils.hpp"

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
	std::signal(SIGINT, handleSigInt);
	webserv.run();
	return (EXIT_SUCCESS);
}
catch (const std::exception& e) {
	return (EXIT_FAILURE);
}
