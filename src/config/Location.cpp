#include <string>
#include <utility>	// make_pair

#include "config/Location.hpp"
#include "webserv_config.hpp"
#include "utils/log.hpp"

namespace	webserv
{

	/**************************************************************************/
	/*                       CONSTRUCTORS / DESTRUCTORS                       */
	/**************************************************************************/

	Location::Location(const Location& src1, const Config& src2,
						const std::string& path)
			: _locationName(path)
			, _maxBodySize(src2.getMaxBodySize()
							? src2.getMaxBodySize()
							: src1._maxBodySize)
			, _limitExcept(src2.getLimitExcept().empty()
							? src1._limitExcept
							: src2.getLimitExcept())
			, _returnPair((src2.getReturnPair().first == -1)
						? std::make_pair(src1._returnPair.first,
										src1._returnPair.second)
						: std::make_pair(src2.getReturnPair().first,
										src2.getReturnPair().second))
			, _errorPages(src2.getErrorPages().empty()
							? src1._errorPages
							: src2.getErrorPages())
			, _root(src2.getRoot().empty() ? src1._root : src2.getRoot())
			, _alias(src2.getAlias().empty() && src2.getRoot().empty()
							? src1._alias
							: src2.getAlias())
			, _index(src2.getIndex().empty() ? src1._index : src2.getIndex())
			, _autoIndex(src2.isAutoIndex() || src1._autoIndex)
			, _cgiPass(src2.getCgiPass().empty()
						? src1._cgiPass
						: src2.getCgiPass())
	{
		LOG_DEBUG("New Location instance (\"" << _locationName << "\")");
	}

	Location::Location(const ServerConfig& src1, const Config& src2,
						const std::string& path)
			: _locationName(path)
			, _maxBodySize(src2.getMaxBodySize()
							? src2.getMaxBodySize()
							: src1.getMaxBodySize())
			, _limitExcept(src2.getLimitExcept())
			, _returnPair((src2.getReturnPair().first == -1)
						? std::make_pair(src1.getReturnPair().first,
										src1.getReturnPair().second)
						: std::make_pair(src2.getReturnPair().first,
										src2.getReturnPair().second))
			, _errorPages(src2.getErrorPages().empty()
							? src1.getErrorPages()
							: src2.getErrorPages())
			, _root(src2.getRoot().empty() ? src1.getRoot() : src2.getRoot())
			, _alias(src2.getAlias())
			, _index(src2.getIndex().empty() ? src1.getIndex() : src2.getIndex())
			, _autoIndex(src2.isAutoIndex() || src1.isAutoIndex())
			, _cgiPass(src2.getCgiPass())
	{
		typedef Config::config_map::const_iterator	map_it;
		
		map_it configIt = src2.getConfigs().begin();
		while (configIt != src2.getConfigs().end()) {
			_locations.insert(std::make_pair(configIt->first,
						Location(*this, configIt->second, configIt->first)));
			configIt++;
		}
		LOG_DEBUG("New Location instance (\"" << _locationName << "\")");
	}

	/**************************************************************************/
	/*                            MEMBER FUNCTIONS                            */
	/**************************************************************************/

}	// namespace webserv
