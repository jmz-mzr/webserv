#include "config/Location.hpp"
#include "utils/Logger.hpp"
#include "webserv_config.hpp"

namespace	webserv
{

	/**************************************************************************/
	/*                       CONSTRUCTORS / DESTRUCTORS                       */
	/**************************************************************************/

	Location::Location(const ServerConfig& src):
											_errorPages(src.getErrorPages()),
											_maxBodySize(src.getMaxBodySize()),
											_autoIndex(false),
											_index("index.html")
	{
		LOG_INFO("New Location instance");
	}

	Location::Location(const Location& src): _errorPages(src._errorPages),
												_maxBodySize(src._maxBodySize),
												_limitExcept(src._limitExcept),
												_return(src._return),
												_root(src._root),
												_autoIndex(src._autoIndex),
												_index(src._index),
												_fastCgiPass(src._fastCgiPass),
												_locations(src._locations)
	{
		LOG_INFO("Location copied");
	}

	/**************************************************************************/
	/*                            MEMBER FUNCTIONS                            */
	/**************************************************************************/

}	// namespace webserv
