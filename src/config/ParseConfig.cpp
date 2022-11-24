#include <string>
#include <iostream>

#include "config/ParseConfig.hpp"
#include "utils/Logger.hpp"
#include "utils/utils.hpp"
#include "webserv_config.hpp"

namespace webserv {

/******************************************************************************/
/*                         CONSTRUCTORS / DESTRUCTORS                         */
/******************************************************************************/

ParseConfig::ParseConfig()
		: _configFilePath(DEFAULT_CONF_FILE)
		, _delimiters("{};#")
		, _nestedBlocks(0)
		, _currentLine(0)
		, _lineBuffer()
{
	_configFile.open(_configFilePath.c_str());
	if (!_configFile.good()) {
		LOG_WARN("Cannot open \"" << _configFilePath << "\"");
		LOG_ERROR("Loading configuration failed")
		throw LogicErrorException();
	}
	_initTokenMap();
	LOG_INFO("New ParseConfig instance with default configuration");
	LOG_DEBUG("_configFilePath=" << _configFilePath);
}

ParseConfig::ParseConfig(const std::string& configFilePath)
		: _configFilePath(configFilePath)
		, _delimiters("{};#")
		, _nestedBlocks(0)
		, _currentLine(0)
		, _lineBuffer()
{
	_configFile.open(_configFilePath.c_str());
	if (!_configFile.good()) {
		LOG_WARN("Cannot open \"" << _configFilePath << "\"");
		_configFilePath = DEFAULT_CONF_FILE;
		_configFile.open(_configFilePath.c_str());
		if (!_configFile.good()) {
			LOG_WARN("Cannot open \"" << _configFilePath << "\"");
			LOG_ERROR("Loading configuration failed")
			throw LogicErrorException();
		}
		LOG_INFO("New ParseConfig instance with default configuration");
	} else {
		LOG_INFO("New ParseConfig instance with provided configuration");
	}
	_initTokenMap();
	LOG_DEBUG("_configFilePath=" << _configFilePath);
}

ParseConfig::~ParseConfig()
{
	_configFile.clear();
	if (_configFile.is_open()) {
		_configFile.close();
		if (!_configFile.good()) {
			LOG_WARN("Bad close() on \"" << _configFilePath << "\"");
		} else {
			LOG_INFO("close(" << _configFilePath << ")");
		}
	}
}

/******************************************************************************/
/*                              MEMBER FUNCTIONS                              */
/******************************************************************************/

void	ParseConfig::_initTokenMap()
{
	_tokenTypes['{'] = kBlockStart;
	_tokenTypes['}'] = kBlockEnd;
	_tokenTypes[';'] = kDirectiveEnd;
	_tokenTypes['#'] = kComment;
}

void	ParseConfig::_syntaxError(const Token& token, const char* expected)
{
	std::stringstream	ss;

	ss << "unexpected ";
	if (token.type == kEOF) {
		ss << "end of file, expecting " << expected;
	} else {
		ss << "\"" << token.value << "\"";
	}
	Logger::getInstance().log(_configFilePath, _currentLine, kEmerg, ss.str());
	throw LogicErrorException();
}

void	ParseConfig::_addToken(const Token& token)
{
	switch (token.type) {
		case kEOF:
			if (!_tokens.empty() && _tokens.back().type == kWord) {
				_syntaxError(token, "\";\" or \"}\"");
			} else if (_nestedBlocks > 0) {
				_syntaxError(token, "\"}\"");
			}
			break ;
		case kBlockStart:
			if (_tokens.empty() || (_tokens.back().type != kWord))
				_syntaxError(token, "");
			_nestedBlocks++;
			break ;
		case kBlockEnd:
			if (!_nestedBlocks)
				_syntaxError(token, "");
			_nestedBlocks--;
			break ;
		case kDirectiveEnd:
			if (_tokens.back().type != kWord)
				_syntaxError(token, "");
			break ;
		default:
			break ;
	}
	_tokens.push_back(token);
}

void	ParseConfig::_extractWords(const std::string& buffer)
{
	std::stringstream	ss(buffer);
	std::string			word;

	while (!ss.eof()) {
		ss >> word;
		if (!word.empty())
			_addToken(Token(kWord, word));
	}
}

void	ParseConfig::_lexer()
{
	size_t						start = 0;
	size_t						pos = 0;
	token_map_t::const_iterator	it;

	do {
		pos = _lineBuffer.find_first_of(_delimiters, start);
		if (pos == std::string::npos) {
			pos = _lineBuffer.find_first_not_of(_delimiters, start);
			if (pos != std::string::npos)
				_extractWords(trim(_lineBuffer));
			break ;
		} else if (pos != start) {
			_extractWords(trim(_lineBuffer.substr(start, pos - start)));
		}
		it = _tokenTypes.find(_lineBuffer[pos]);
		if (it->first == '#')
			break ;
		_addToken(Token(it->second, std::string(1, it->first)));
		start = ++pos;
	} while (pos != std::string::npos);
	if (_configFile.eof())
		_addToken(Token(kEOF, "EOF"));
}

void ParseConfig::_parser()
{
}

void    ParseConfig::_readLine()
{
	std::getline(_configFile, _lineBuffer);
	_currentLine++;
}

void    ParseConfig::operator()()
{
	do {
		_readLine();
		if (!_lineBuffer.empty() || _configFile.eof())
		{
			_lexer();
			_parser();
		}
	} while (!_configFile.eof());
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
/*                          NON-MEMBER FUNCTIONS                              */
/******************************************************************************/

std::ostream&	operator<<(std::ostream& os,
							const ParseConfig::token_list_t& rhs)
{
	typedef std::vector<struct ParseConfig::Token>::const_iterator cIter_t;

	for (cIter_t currToken = rhs.begin(); currToken != rhs.end(); currToken++) {
		switch (currToken->type) {
			case kEOF:
				os << HRED;
				break ;
			case kWord:
				os << HBLU;
				break ;
			case kBlockStart:
			case kBlockEnd:
				os << HGRN;
				break ;
			case kDirectiveEnd:
				os << HYEL;
				break ;
			default :
				break;
		}
		os << currToken->value;
		if (currToken->type != kEOF)
			os << HWHT << " -> ";
		os << RESET;
	}
	os << std::endl;
	return os;
}

}	// namespace webserv