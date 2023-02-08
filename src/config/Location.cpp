#include "config/Location.hpp"
#include "utils/Logger.hpp"
#include "webserv_config.hpp"

namespace	webserv
{

	/**************************************************************************/
	/*                       CONSTRUCTORS / DESTRUCTORS                       */
	/**************************************************************************/

	Location::Location(const Location& src)
			: _locationName(src._locationName)
			, _maxBodySize(src._maxBodySize)
			, _limitExcept(src._limitExcept)
			, _return(src._return)
			, _errorPages(src._errorPages)
			, _root(src._root)
			, _alias(src._alias)
			, _index(src._index)
			, _autoIndex(src._autoIndex)
			, _cgiPass(src._cgiPass)
			, _locations(src._locations)
	{ }

	Location::Location(const ServerConfig& src1, const Config& src2,
						const std::string& path)
			: _locationName(path)
			, _maxBodySize(src1.getMaxBodySize())
			, _limitExcept(src2.getLimitExcept())
			, _return(src2.getReturnPair())
			, _errorPages(src1.getErrorPages())
			, _root(src2.getRoot())
			, _alias(src2.getAlias())
			, _index(src2.getIndex())
			, _autoIndex(src2.isAutoIndex())
			, _cgiPass(src2.getCgiPass())
	{ }

	/**************************************************************************/
	/*                            MEMBER FUNCTIONS                            */
	/**************************************************************************/

}	// namespace webserv
