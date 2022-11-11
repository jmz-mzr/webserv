#include "config/Config.hpp"
#include "utils/Logger.hpp"
#include "webserv_config.hpp"

#include <algorithm>

namespace	webserv
{

	/**************************************************************************/
	/*                       CONSTRUCTORS / DESTRUCTORS                       */
	/**************************************************************************/

	Config::Config()
	{
		LOG_INFO("New Config instance");
	}

	/**************************************************************************/
	/*                            MEMBER FUNCTIONS                            */
	/**************************************************************************/

	void	Config::openConfigFile(const std::string& configFileName)
	{
		_configFileName = configFileName;
		_configFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
		try {
			_configFile.open(configFileName.c_str());
		} catch (std::ifstream::failure &e) {
			if (configFileName == DEFAULT_CONF_FILE) {
				LOG_DEBUG("Cannot open \"" << configFileName << "\"");
				throw ;
			} else {
				LOG_WARN("Cannot open \"" << configFileName
						<< "\", continue with default config file \""
						<< DEFAULT_CONF_FILE << "\"");
				_configFile.clear();
				try {
					_configFile.open(DEFAULT_CONF_FILE);
				} catch (std::ifstream::failure &e) {
					LOG_DEBUG("Cannot open \"" << DEFAULT_CONF_FILE << "\"");
					throw ;
				}
			}
		}
	}

	void	Config::parseConfig()
	{
		ServerConfig	serverConfig;

		while (1) {
			serverConfig.clearConfig();
			// TO DO: actual parsing of every Server block
			serverConfig.addListenPair(std::make_pair("127.0.0.1", 8081));
			serverConfig.addName("webserv");
			_serverConfigs.push_back(serverConfig);
			break ;
		}
		if (_serverConfigs.empty())
			throw FatalErrorException("The config file has no server block");
	}

	void	Config::closeConfigFile()
	{
		if (_configFile.is_open()) {
			try {
				_configFile.close();
			} catch (std::ifstream::failure &e) {
				LOG_WARN("Bad close() on \"" << _configFileName << "\"");
				return ;
			}
			LOG_INFO("close(" << _configFileName << ")");
		}
	}

}	// namespace webserv
