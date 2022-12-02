#ifdef DOCTEST_CONFIG_DISABLE	// Compile without tests

# include <cerrno>
# include <csignal>
# include <cstdlib>
# include <iostream>

# include "core/Webserv.hpp"
# include "utils/exceptions.hpp"
# include "utils/Logger.hpp"

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
}
catch (const webserv::FatalErrorException& e) {
	LOG_ERROR(e.what());
	return (EXIT_FAILURE);
}
catch (const webserv::LogicErrorException& e) {
	LOG_EMERG(e.what());
	return (EXIT_FAILURE);
}
catch (...) {
	return (EXIT_FAILURE);
}

#else	// Generate a main function for testing

# define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
# define DOCTEST_CONFIG_SUPER_FAST_ASSERTS //! undef if using debugger
# include "doctest/doctest.h"

#endif
