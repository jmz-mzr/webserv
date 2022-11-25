#include "config/Lexer.hpp"

#include <deque>
#include <string>
#include <iostream>

#include "config/ConfigParser.hpp"
#include "utils/ansi_colors.hpp"
#include "utils/Logger.hpp"
#include "utils/utils.hpp"

namespace webserv {

namespace config {

/******************************************************************************/
/*                         CONSTRUCTORS / DESTRUCTORS                         */
/******************************************************************************/

Lexer::Lexer(const ConfigParser& src)
		: _delimiters("{};#")
		, _tokens()
		, _nestedBlockCount(0)
		, _configParser(src)
{
	_tokenTypes['{'] = kBlockStart;
	_tokenTypes['}'] = kBlockEnd;
	_tokenTypes[';'] = kDirectiveEnd;
	_tokenTypes['#'] = kComment;
}

Lexer::~Lexer() { }

/******************************************************************************/
/*                              MEMBER FUNCTIONS                              */
/******************************************************************************/

void	Lexer::_syntaxError(const Token& token, const char* expected)
{
	std::stringstream	ss;

	ss << "unexpected ";
	if (token.type == kEOF) {
		ss << "end of file, expecting " << expected;
	} else {
		ss << "\"" << token.value << "\"";
	}
	Logger::getInstance().log(_configParser._filePath,
								_configParser._currentLineNb, kEmerg, ss.str());
	throw LogicErrorException();
}

void	Lexer::_addToken(const Token& token)
{
	switch (token.type) {
		case kEOF:
			if (!_tokens.empty() && _tokens.back().type == kWord) {
				_syntaxError(token, "\";\" or \"}\"");
			} else if (_nestedBlockCount > 0) {
				_syntaxError(token, "\"}\"");
			}
			break ;
		case kBlockStart:
			if (_tokens.empty() || (_tokens.back().type != kWord))
				_syntaxError(token, "");
			_nestedBlockCount++;
			break ;
		case kBlockEnd:
			if (!_nestedBlockCount)
				_syntaxError(token, "");
			_nestedBlockCount--;
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

void	Lexer::_extractWords(const std::string& buffer)
{
	std::stringstream	ss(buffer);
	std::string			word;

	while (!ss.eof()) {
		ss >> word;
		if (!word.empty())
			_addToken(Token(kWord, word));
	}
}

void	Lexer::operator()()
{
	size_t						start = 0;
	size_t						pos = 0;
	token_dict::const_iterator	it;

	do {
		pos = _configParser._lineBuffer.find_first_of(_delimiters, start);
		if (pos == std::string::npos) {
			pos = _configParser._lineBuffer.find_first_not_of(_delimiters, start);
			if (pos != std::string::npos)
				_extractWords(trim(_configParser._lineBuffer));
			break ;
		} else if (pos != start) {
			_extractWords(trim(_configParser._lineBuffer.substr(start, pos - start)));
		}
		it = _tokenTypes.find(_configParser._lineBuffer[pos]);
		if (it->first == '#')
			break ;
		_addToken(Token(it->second, std::string(1, it->first)));
		start = ++pos;
	} while (pos != std::string::npos);
	if (_configParser._file.eof())
		_addToken(Token(kEOF, "EOF"));
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

std::ostream&	operator<<(std::ostream& os, const Lexer::token_queue& rhs)
{
	typedef std::deque<struct Lexer::Token>::const_iterator const_iter;

	for (const_iter currentToken = rhs.begin();
					currentToken != rhs.end();
					currentToken++) {
		switch (currentToken->type) {
			case Lexer::kEOF:
				os << HRED;
				break ;
			case Lexer::kWord:
				os << HBLU;
					break ;
				case Lexer::kBlockStart:
				case Lexer::kBlockEnd:
					os << HGRN;
					break ;
				case Lexer::kDirectiveEnd:
					os << HYEL;
					break ;
				default :
					break;
			}
		os << currentToken->value;
		if (currentToken->type != Lexer::kEOF)
			os << HWHT << " -> ";
		os << RESET;
	}
	return os;
}

}	// namespace config

}	// namespace webserv
