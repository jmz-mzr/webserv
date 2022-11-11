#include "config/ServerConfig.hpp"
#include "utils/Logger.hpp"

#include <algorithm>

namespace	webserv
{

	/**************************************************************************/
	/*                       CONSTRUCTORS / DESTRUCTORS                       */
	/**************************************************************************/

	ServerConfig::ServerConfig()
	{
		LOG_INFO("New ServerConfig instance");
	}

	ServerConfig::ServerConfig(const ServerConfig& src):
												_listenPairs(src._listenPairs),
												_names(src._names)
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
		if (std::find(_names.begin(), _names.end(), name) != _names.end()) {
			LOG_INFO("Name already in ServerConfig");
		} else {
			_names.push_back(name);
			LOG_INFO("Name added to ServerConfig");
		}
		LOG_DEBUG("name=" << name);
	}

	bool	ServerConfig::eraseName(const std::string& name)
	{
		std::vector<std::string>::iterator	nameToDel;

		nameToDel = std::find(_names.begin(), _names.end(), name);
		if (nameToDel != _names.end()) {
			_names.erase(nameToDel);
			return (true);
		}
		return (false);
	}

	void	ServerConfig::clearConfig()
	{
		_listenPairs.clear();
		_names.clear();
	}

}	// namespace webserv
