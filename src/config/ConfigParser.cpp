#include <string>
#include <iostream>

#include "config/ConfigParser.hpp"
#include "utils/Logger.hpp"
#include "utils/utils.hpp"
#include "webserv_config.hpp"

namespace webserv {

namespace config {

/******************************************************************************/
/*                         CONSTRUCTORS / DESTRUCTORS                         */
/******************************************************************************/

ConfigParser::ConfigParser()
		: _filePath(DEFAULT_CONF_FILE)
		, _currentLineNb(0)
		, _lexer(*this)
		, _maxBodySize(0)
{
	_file.open(_filePath.c_str());
	if (!_file.good()) {
		LOG_WARN("Cannot open \"" << _filePath << "\"");
		LOG_ERROR("Loading configuration failed")
		throw LogicErrorException();
	}
	LOG_INFO("New ConfigParser instance with default configuration");
	LOG_DEBUG("_filePath=" << _filePath);
}

ConfigParser::ConfigParser(const std::string& path)
		: _filePath(path)
		, _currentLineNb(0)
		, _lexer(*this)
		, _maxBodySize(0)
{
	_file.open(_filePath.c_str());
	if (!_file.good()) {
		LOG_WARN("Cannot open \"" << _filePath << "\"");
		_filePath = DEFAULT_CONF_FILE;
		_file.open(_filePath.c_str());
		if (!_file.good()) {
			LOG_WARN("Cannot open \"" << _filePath << "\"");
			LOG_ERROR("Loading configuration failed")
			throw LogicErrorException();
		}
		LOG_INFO("New ConfigParser instance with default configuration");
	} else {
		LOG_INFO("New ConfigParser instance with provided configuration");
	}
	LOG_DEBUG("_filePath=" << _filePath);
}

ConfigParser::~ConfigParser()
{
	_file.clear();
	if (_file.is_open()) {
		_file.close();
		if (!_file.good()) {
			LOG_WARN("Bad close() on \"" << _filePath << "\"");
		} else {
			LOG_INFO("close(" << _filePath << ")");
		}
	}
}

/******************************************************************************/
/*                              MEMBER FUNCTIONS                              */
/******************************************************************************/

void	ConfigParser::operator()()
{
	do {
		std::getline(_file, _lineBuffer);
		_currentLineNb++;
		if (!_lineBuffer.empty() || _file.eof()) {
			_lexer();
			// _parser();
		}
	} while (!_file.eof());
}

// ServerConfig	serverConfig(*this);

// TODO: actual parsing of every Server block
// serverConfig.addListenPair(std::make_pair("127.0.0.1", 8081));
// serverConfig.addName("webserv");
// _serverConfigs.push_back(serverConfig);

// error_pages_map					_errorPages;
// long long						_maxBodySize;
// std::vector<ServerConfig>		_serverConfigs;

/******************************************************************************/
/*                            NON-MEMBER FUNCTIONS                            */
/******************************************************************************/

}	// namespace config

}	// namespace webserv
