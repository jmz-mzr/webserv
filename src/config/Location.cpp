#include "config/Location.hpp"
#include "utils/Logger.hpp"
#include "webserv_config.hpp"

namespace	webserv
{

	/**************************************************************************/
	/*                       CONSTRUCTORS / DESTRUCTORS                       */
	/**************************************************************************/

	Location::Location(const Location& src)
			: _maxBodySize(src._maxBodySize)
			, _limitExcept(src._limitExcept)
			, _return(src._return)
			, _fastCgiPass(src._fastCgiPass)
			, _index(src._index)
			, _autoIndex(src._autoIndex)
			, _root(src._root)
			, _errorPages(src._errorPages)
			, _locations(src._locations)
	{ }

	Location::Location(const ServerConfig& src1, const Config& src2)
			: _maxBodySize(src1.getMaxBodySize())
			, _limitExcept(src2.getLimitExcept())
			, _return(src2.getReturnPair())
			, _fastCgiPass(src2.getFastCgiPass())
			, _index(src2.getIndex())
			, _autoIndex(src2.isAutoIndex())
			, _root(src2.getRoot())
			, _errorPages(src1.getErrorPages())
	{ }

	/**************************************************************************/
	/*                            MEMBER FUNCTIONS                            */
	/**************************************************************************/

}	// namespace webserv
