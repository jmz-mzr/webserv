#include <string>

#include "config/ParseConfig.hpp"
#include "utils/Logger.hpp"
#include "utils/utils.hpp"
#include "webserv_config.hpp"

namespace webserv {

/******************************************************************************/
/*                         CONSTRUCTORS / DESTRUCTORS                         */
/******************************************************************************/

ParseConfig::ParseConfig(const std::string& configFilePath)
        : _configFilePath(configFilePath)
{
    _configFile.open(_configFilePath.c_str());
    if (!_configFile.good()) {
        LOG_WARN("Cannot open \"" << _configFilePath << "\"");
        if (_configFilePath == DEFAULT_CONF_FILE) {
            throw LogicErrorException("Could not load configuration");
        } else {
            _configFilePath = DEFAULT_CONF_FILE;
            _configFile.clear();
            _configFile.open(_configFilePath.c_str());
            if (!_configFile.good()) {
                LOG_WARN("Cannot open \"" << _configFilePath << "\"");
                throw LogicErrorException("Could not load configuration");
            }
        }
    }
    LOG_INFO("New ParseConfig instance");
}

ParseConfig::~ParseConfig()
{
    _configFile.close();
    if (!_configFile.good()) {
        LOG_WARN("Bad close() on \"" << _configFilePath << "\"");
    } else {
        LOG_INFO("close(" << _configFilePath << ")");
    }
}

/******************************************************************************/
/*                              MEMBER FUNCTIONS                              */
/******************************************************************************/

}	// namespace webserv