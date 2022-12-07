#include "config/ServerConfig.hpp"

#include <algorithm>

#include "config/ConfigParser.hpp"
#include "utils/Logger.hpp"

namespace	webserv
{

	/**************************************************************************/
	/*                       CONSTRUCTORS / DESTRUCTORS                       */
	/**************************************************************************/

	ServerConfig::ServerConfig(const Config& src, const Address& listenPair)
		: _listenPair(listenPair)
		, _serverNames(src.getServerNames().begin(), src.getServerNames().end())
		, _errorPages(src.getErrorPages())
		, _maxBodySize(src.getMaxBodySize())
	{
		typedef Config::config_map::const_iterator	map_it;

		if (_serverNames.empty())
			_serverNames.insert("");
		map_it configIt = src.getConfigs().begin();
		while (configIt != src.getConfigs().end()) {
			map_it configIt2 = configIt->second.getConfigs().begin();
			while (configIt2 != configIt->second.getConfigs().end()) {
				_locations.insert(std::make_pair(configIt2->first,
								Location(*this, configIt2->second)));
				configIt2++;
			}
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
