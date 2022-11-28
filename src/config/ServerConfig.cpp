#include "config/ServerConfig.hpp"

#include <algorithm>

#include "config/ConfigParser.hpp"
#include "utils/Logger.hpp"

namespace	webserv
{

	/**************************************************************************/
	/*                       CONSTRUCTORS / DESTRUCTORS                       */
	/**************************************************************************/

	ServerConfig::ServerConfig(const config::ConfigParser& src):
											_errorPages(src.getErrorPages()),
											_maxBodySize(src.getMaxBodySize())
	{
		LOG_INFO("New ServerConfig instance");
		_locations.insert(std::make_pair("", Location(*this)));
	}

	ServerConfig::ServerConfig(const ServerConfig& src):
												_listenPairs(src._listenPairs),
												_serverNames(src._serverNames),
												_errorPages(src._errorPages),
												_maxBodySize(src._maxBodySize),
												_locations(src._locations)
	{
		LOG_INFO("ServerConfig copied");
	}

	/**************************************************************************/
	/*                            MEMBER FUNCTIONS                            */
	/**************************************************************************/

	void	ServerConfig::addListenPair(const listen_pair& listenPair)
	{
		if (std::find(_listenPairs.begin(), _listenPairs.end(),
					listenPair) != _listenPairs.end()) {
			LOG_INFO("ListenPair already in ServerConfig");
		} else {
			_listenPairs.push_back(listenPair);
			LOG_INFO("ListenPair added to ServerConfig");
		}
		LOG_DEBUG("addr=" << listenPair.first << " ; "
				<< "port=" << listenPair.second);
	}

	void	ServerConfig::addName(const std::string& name)
	{
		if (std::find(_serverNames.begin(), _serverNames.end(), name)
				!= _serverNames.end()) {
			LOG_INFO("Name already in ServerConfig");
		} else {
			_serverNames.push_back(name);
			LOG_INFO("Name added to ServerConfig");
		}
		LOG_DEBUG("name=" << name);
	}

	bool	ServerConfig::eraseName(const std::string& name)
	{
		std::vector<std::string>::iterator	nameToDel;

		nameToDel = std::find(_serverNames.begin(), _serverNames.end(), name);
		if (nameToDel != _serverNames.end()) {
			_serverNames.erase(nameToDel);
			return (true);
		}
		return (false);
	}

}	// namespace webserv
