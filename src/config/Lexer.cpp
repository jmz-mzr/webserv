#include "config/Lexer.hpp"

#include <deque>
#include <string>
#include <iostream>

#include "config/ConfigParser.hpp"
#include "utils/ansi_colors.hpp"
#include "utils/exceptions.hpp"
#include "utils/Logger.hpp"
#include "utils/utils.hpp"

namespace webserv {

namespace config {

/******************************************************************************/
/*                         CONSTRUCTORS / DESTRUCTORS                         */
/******************************************************************************/

Lexer::Lexer()
		: _delimiters("{};#")
		, _tokens()
		, _nestedBlockCount(0)
{
	_tokenTypes['{'] = Token::kBlockStart;
	_tokenTypes['}'] = Token::kBlockEnd;
	_tokenTypes[';'] = Token::kDirectiveEnd;
	_tokenTypes['#'] = Token::kComment;
}

Lexer::~Lexer() { }

/******************************************************************************/
/*                              MEMBER FUNCTIONS                              */
/******************************************************************************/

void	Lexer::_syntaxError(const Token& token, const char* expected)
{
	std::stringstream	ss;

	ss << "unexpected ";
	if (token.type == Token::kEOF) {
		ss << "end of file, expecting " << expected;
	} else {
		ss << "\"" << token.value << "\"";
	}
	throw SyntaxErrorException(ss.str());
}

void	Lexer::_addToken(const Token& token)
{
	switch (token.type) {
		case Token::kEOF:
			if (!_tokens.empty() && _tokens.back().type == Token::kWord) {
				_syntaxError(token, "\";\" or \"}\"");
			} else if (_nestedBlockCount > 0) {
				_syntaxError(token, "\"}\"");
			}
			break ;
		case Token::kBlockStart:
			if (_tokens.empty() || (_tokens.back().type != Token::kWord))
				_syntaxError(token, "");
			_nestedBlockCount++;
			break ;
		case Token::kBlockEnd:
			if (!_nestedBlockCount)
				_syntaxError(token, "");
			_nestedBlockCount--;
			break ;
		case Token::kDirectiveEnd:
			if (_tokens.back().type != Token::kWord)
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
			_addToken(Token(Token::kWord, word));
	}
}

void	Lexer::operator()(const std::string& lineBuffer)
{
	size_t						start = 0;
	size_t						pos = 0;
	token_dict::const_iterator	it;

	do {
		pos = lineBuffer.find_first_of(_delimiters, start);
		if (pos == std::string::npos) {
			pos = lineBuffer.find_first_not_of(_delimiters, start);
			if (pos != std::string::npos)
				_extractWords(trim(lineBuffer));
			break ;
		} else if (pos != start)
			_extractWords(trim(lineBuffer.substr(start, pos - start)));
		it = _tokenTypes.find(lineBuffer[pos]);
		if (it->second == Token::kComment)
			break ;
		_addToken(Token(it->second, std::string(1, it->first)));
		start = ++pos;
	} while (pos != std::string::npos);
	if (isEof)
		_addToken(Token(Token::kEOF, "EOF"));
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
			case Lexer::Token::kEOF:
				os << HRED;
				break ;
			case Lexer::Token::kWord:
				os << HBLU;
					break ;
				case Lexer::Token::kBlockStart:
				case Lexer::Token::kBlockEnd:
					os << HGRN;
					break ;
				case Lexer::Token::kDirectiveEnd:
					os << HYEL;
					break ;
				default :
					break;
			}
		os << currentToken->value;
		if (currentToken->type != Lexer::Token::kEOF)
			os << HWHT << " -> ";
		os << RESET;
	}
	return os;
}

}	// namespace config

}	// namespace webserv
