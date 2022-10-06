#ifdef DOCTEST_CONFIG_DISABLE	// Compile without tests

# include "config.hpp"

int	main( void )
{
	return (0);
}

#else							// Generate a main function for testing

# define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
# include "config.hpp"

#endif