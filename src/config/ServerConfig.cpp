#include "config/ServerConfig.hpp"

#include <algorithm>
#include <cstring>

#include "config/ConfigParser.hpp"
#include "utils/log.hpp"
#include "utils/utils.hpp"

namespace	webserv
{

	/**************************************************************************/
	/*                       CONSTRUCTORS / DESTRUCTORS                       */
	/**************************************************************************/

	ServerConfig::ServerConfig(const Config& src, const sockaddr_in& listenPair)
		: _serverNames(src.getServerNames().begin(), src.getServerNames().end())
		, _errorPages(src.getErrorPages())
		, _maxBodySize(src.getMaxBodySize() ? src.getMaxBodySize() : (1 << 20))
		, _return(src.getReturnPair())
		, _root(src.getRoot().empty() ? "html" : src.getRoot())
		, _autoIndex(src.isAutoIndex())
		, _index(src.getIndex().empty() ? "index.html" : src.getIndex())
	{
		typedef Config::config_map::const_iterator	map_it;

		copySockAddr(_listenPair, listenPair);
		if (_serverNames.empty())
			_serverNames.insert("");
		map_it configIt = src.getConfigs().begin();
		while (configIt != src.getConfigs().end()) {
			_locations.insert(std::make_pair(configIt->first,
								Location(*this, configIt->second,
								configIt->first)));
			configIt++;
		}
		LOG_INFO("New ServerConfig instance");
	}

	/**************************************************************************/
	/*                            MEMBER FUNCTIONS                            */
	/**************************************************************************/

	bool	ServerConfig::eraseName(const std::string& name)
	{
		ServerConfig::hostname_set::iterator	nameToDel;

		nameToDel = std::find(_serverNames.begin(), _serverNames.end(), name);
		if (nameToDel != _serverNames.end()) {
			_serverNames.erase(nameToDel);
			return (true);
		}
		return (false);
	}

}	// namespace webserv
