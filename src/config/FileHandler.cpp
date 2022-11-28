#include "config/FileHandler.hpp"

#include <string>
#include <iostream>

#include "utils/Logger.hpp"
#include "utils/utils.hpp"
#include "webserv_config.hpp"

namespace webserv {

namespace	config {

/******************************************************************************/
/*                         CONSTRUCTORS / DESTRUCTORS                         */
/******************************************************************************/

FileHandler::FileHandler()
		: _filePath(DEFAULT_CONF_FILE)
		, _currentLineNb(0)
		, _lexer()
{
	_file.open(_filePath.c_str());
	if (!_file.good()) {
		LOG_WARN("Cannot open \"" << _filePath << "\"");
		LOG_ERROR("Loading configuration failed")
		throw LogicErrorException();
	}
	LOG_INFO("New FileHandler instance with default configuration");
	LOG_DEBUG("_filePath=" << _filePath);
}

FileHandler::FileHandler(const std::string& path)
		: _filePath(path)
		, _currentLineNb(0)
		, _lexer()
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
		LOG_INFO("New FileHandler instance with default configuration");
	} else {
		LOG_INFO("New FileHandler instance with provided configuration");
	}
	LOG_DEBUG("_filePath=" << _filePath);
}

FileHandler::~FileHandler()
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

bool	FileHandler::_readline()
{
	std::getline(_file, _lineBuffer);
	if (_file.fail())
		throw FatalErrorException("Error while reading configuration file");
	_currentLineNb++;
	return ((_lexer.isEof = _file.eof()) || !_lineBuffer.empty());
}

void	FileHandler::parse()
{
	token_queue		tokens;

	try {
		do {
			if (_readline() == true) {
				_lexer.isParseReady = false;
				_lexer(_lineBuffer);
				if (_lexer.isParseReady == true)
					_parser(_lexer.getTokens());
			}
		} while (_file.good());
	} catch (const SyntaxErrorException& e) {
		Logger::getInstance().log(_filePath, _currentLineNb, LogLevel::kEmerg,
																	e.what());
		throw ;
	} catch (...) {
		throw ;
	}
}

// ServerFileHandler	serverFileHandler(*this);

// TODO: actual parsing of every Server block
// serverFileHandler.addListenPair(std::make_pair("127.0.0.1", 8081));
// serverFileHandler.addName("webserv");
// _serverFileHandlers.push_back(serverFileHandler);

// error_pages_map					_errorPages;
// long long						_maxBodySize;
// std::vector<ServerFileHandler>		_serverFileHandlers;

/******************************************************************************/
/*                            NON-MEMBER FUNCTIONS                            */
/******************************************************************************/

}	// namespace config

}	// namespace webserv
