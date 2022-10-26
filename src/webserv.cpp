#ifdef DOCTEST_CONFIG_DISABLE	// Compile without tests

#include <iostream>
#include <cstdlib>
#include <csignal>

#include "utils/Logger.hpp"
#include "core/Server.hpp"

static void	handleSigint(int signum)
{
	(void)signum;
	std::cout << "\r\033[2K";
	LOG_INFO("SIGINT received");
}

int	main(int argc, char **argv)
{
	if (argc != 2) {
		LOG_ERROR("Bad number of arguments");
		exit(EXIT_FAILURE);
	}

	signal(SIGINT, handleSigint);
	webserv::Server server("127.0.0.1", (uint16_t)atoi(argv[1]));
	(void)argv;

	int ret = server.eventLoop();

	return ret;
}

#else	// Generate a main function for testing

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest/doctest.h"

#endif