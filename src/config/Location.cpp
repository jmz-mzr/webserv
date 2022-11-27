#include "config/Location.hpp"
#include "utils/Logger.hpp"
#include "webserv_config.hpp"

namespace	webserv
{

	/**************************************************************************/
	/*                       CONSTRUCTORS / DESTRUCTORS                       */
	/**************************************************************************/

	Location::Location(const ServerConfig& src):
											_maxBodySize(src.getMaxBodySize()),
											_index("index.html"),
											_autoIndex(false),
											_errorPages(src.getErrorPages())
	{
		LOG_INFO("New Location instance");
	}

	Location::Location(const Location& src): _maxBodySize(src._maxBodySize),
												_limitExcept(src._limitExcept),
												_return(src._return),
												_fastCgiPass(src._fastCgiPass),
												_index(src._index),
												_autoIndex(src._autoIndex),
												_root(src._root),
												_errorPages(src._errorPages),
												_locations(src._locations)
	{
		LOG_INFO("Location copied");
	}

	/**************************************************************************/
	/*                            MEMBER FUNCTIONS                            */
	/**************************************************************************/

}	// namespace webserv
