#include <stdint.h>		// uint32_t

#include <list>
#include <string>		// getline

#include "config/ConfigParser.hpp"
#include "webserv_config.hpp"
#include "utils/exceptions.hpp"
#include "utils/global_defs.hpp"
#include "utils/log.hpp"
#include "utils/utils.hpp"

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
		if (_filePath == XSTR(CONF_FILE)) {
			LOG_ERROR("Cannot open the default \"" << _filePath << "\" file");
			THROW_LOGIC("Loading configuration failed");
		}
		LOG_WARN("Cannot open \"" << _filePath << "\"");
		_filePath = XSTR(CONF_FILE);
		_file.open(_filePath.c_str());
		if (!_file.good()) {
			LOG_ERROR("Cannot open the default \"" << _filePath << "\" file");
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
		Log::Core::get().filter(Log::ContextInfo(_filePath, _currentLineNb,
												Log::Level::kEmerg), e.what());
		throw ;
	} catch (...) {
		throw ;
	}
}

/******************************************************************************/
/*                            NON-MEMBER FUNCTIONS                            */
/******************************************************************************/

}	// namespace webserv
