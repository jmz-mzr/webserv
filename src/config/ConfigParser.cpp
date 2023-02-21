#include <string>
#include <iostream>

#include "config/ConfigParser.hpp"
#include "utils/exceptions.hpp"
#include "utils/global_defs.hpp"
#include "utils/Logger.hpp"
#include "utils/utils.hpp"
#include "webserv_config.hpp"

namespace webserv {

/******************************************************************************/
/*                         CONSTRUCTORS / DESTRUCTORS                         */
/******************************************************************************/

ConfigParser::ConfigParser(const std::string& path)
		: _filePath(path)
		, _currentLineNb(0)
		, _lexer()
		, _parser()
{
	_file.open(_filePath.c_str());
	if (!_file.good()) {
		LOG_WARN("Cannot open \"" << _filePath << "\"");
		if (_filePath == XSTR(CONF_FILE))
			THROW_LOGIC("Loading configuration failed");
		_filePath = XSTR(CONF_FILE);
		_file.open(_filePath.c_str());
		if (!_file.good()) {
			LOG_WARN("Cannot open \"" << _filePath << "\"");
			THROW_LOGIC("Loading configuration failed");
		}
	}
	if (_filePath == XSTR(CONF_FILE)) {
		LOG_INFO("Default configuration file opened [" << _filePath << "]");
	} else {
		LOG_INFO("Provided configuration file opened [" << _filePath << "]");
	}
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

const std::ifstream&				ConfigParser::getFile() const
{ return (_file); }

const std::string&					ConfigParser::getFilePath() const
{ return (_filePath); }

const uint32_t&						ConfigParser::getCurrentLineNb() const
{ return (_currentLineNb); }

bool								ConfigParser::_readline()
{
	std::getline(_file, _lineBuffer);
	if (_file.bad())
		THROW_FATAL("Error while reading configuration file");
	_currentLineNb++;
	return ((_lexer.isEof = _file.eof()) || !_lineBuffer.empty());
}

const std::list<Config>&			ConfigParser::parseFile()
{
	try {
		do {
			if (_readline() == true) {
				_lexer(_lineBuffer);
				_parser(_lexer.getTokens());
			}
		} while (_file.good());
		return (_parser.getConfigs());
	} catch (const SyntaxErrorException& e) {
		Logger::getInstance().log(_filePath, _currentLineNb,
										Logger::kEmerg, e.what());
		throw ;
	} catch (...) {
		throw ;
	}
}

/******************************************************************************/
/*                            NON-MEMBER FUNCTIONS                            */
/******************************************************************************/

}	// namespace webserv
