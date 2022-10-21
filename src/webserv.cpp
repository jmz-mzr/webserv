#ifdef DOCTEST_CONFIG_DISABLE	// Compile without tests

# include "webserv.hpp"

static void	handle_sigint(int signum)
{
	//clean();
	exit(signum);
}

int	main(int argc, char **argv)
{
	if ( argc != 2 )
	{
		LOG(wsrv::LL_ERROR, "Bad number of arguments");
		exit(EXIT_FAILURE);
	}

	signal(SIGINT, handle_sigint);
//	wsrv::Server server("127.0.0.1", (uint16_t)atoi(argv[1]));
	(void)argv;

	// event_loop();
	// clean();
	return (EXIT_SUCCESS);
}

#else							// Generate a main function for testing

# define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
# include "webserv.hpp"

#endif