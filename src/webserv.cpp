#ifdef DOCTEST_CONFIG_DISABLE	// Compile without tests

#include <cstdlib>
#include <csignal>

#include "utils/Logger.hpp"

static void	handle_sigint(int signum)
{
	//clean();
	exit(signum);
}

int	main(int argc, char **argv)
{
	if ( argc != 2 )
	{
		LOG(webserv::kError, "Bad number of arguments");
		exit(EXIT_FAILURE);
	}

	signal(SIGINT, handle_sigint);
//	webserv::Server server("127.0.0.1", (uint16_t)atoi(argv[1]));
	(void)argv;

	// event_loop();
	// clean();
	return (EXIT_SUCCESS);
}

#else	// Generate a main function for testing

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest/doctest.h"

#endif