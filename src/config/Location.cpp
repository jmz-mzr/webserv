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
											_maxBodySize(src.getMaxBodySize())
	{
		LOG_INFO("New Location instance");
	}

	Location::Location(const Location& src): _errorPages(src._errorPages),
												_maxBodySize(src._maxBodySize),
												_limitExcept(src._limitExcept),
												_return(src._return),
												_root(src._root),
												_autoIndex(src._autoIndex),
												_tryFile(src._tryFile),
												_fastCgiPass(src._fastCgiPass)
	{
		LOG_INFO("Location copied");
	}

	/**************************************************************************/
	/*                       MEMBER OPERATOR OVERLOADS                        */
	/**************************************************************************/

/*	Location&	Location::operator=(const Location& rhs)
	{
		// TO DO: Make the _errorPages with "const std::string", _limitExcept
		// with "const int", and _return with "const string"?

		if (this != &rhs) {
			_errorPages = rhs._errorPages;
			_maxBodySize = rhs._maxBodySize;
			_limitExcept = rhs._limitExcept;
			_return = rhs._return;
			_root = rhs._root;
			_autoIndex = rhs._autoIndex;
			_tryFile = rhs._tryFile;
			_fastCgiPass = rhs._fastCgiPass;
		}
		return (*this);
	}*/

	/**************************************************************************/
	/*                            MEMBER FUNCTIONS                            */
	/**************************************************************************/

}	// namespace webserv
