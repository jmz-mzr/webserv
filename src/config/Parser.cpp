#include "config/Parser.hpp"

#include <algorithm>
#include <sstream>
#include <string>

#include "utils/exceptions.hpp"
#include "utils/Logger.hpp"

namespace	webserv {

namespace	config {

Parser::Parser()
{
	Directive	directives[12] = {
		{
			kErrorPage,
			kIgnoreDup | kServCtx,
			2,
			"error_page"
		},
		{
			kMaxBodySize,
			kForbiddenDup | kArgcStrict | kServCtx,
			1,
			"client_max_body_size"
		},
		{
			kLimitExcept,
			kIgnoreDup | kLocCtx,
			1,
			"limit_except"
		},
		{
			kReturn,
			kIgnoreDup | kLocCtx,
			1,
			"return"
		},
		{
			kRoot,
			kForbiddenDup | kArgcStrict | kLocCtx,
			1,
			"root"
		},
		{
			kAutoindex,
			kForbiddenDup | kArgcStrict | kLocCtx,
			1,
			"autoindex"
		},
		{
			kIndex,
			kIgnoreDup | kArgcStrict | kLocCtx,
			1,
			"index"
		},
		{
			kFastCgiPass,
			kForbiddenDup | kArgcStrict | kLocCtx,
			1,
			"fastcgi_pass"
		},
		{
			kServerName,
			kAcceptDup | kServCtx,
			1,
			"server_name"
		},
		{
			kListen,
			kAcceptDup | kArgcStrict | kServCtx,
			1,
			"listen"
		},
		{
			kServer,
			kAcceptDup | kArgcStrict | kNoCtx,
			0,
			"server"
		},
		{
			kLocation,
			kForbiddenDup | kArgcStrict | kServCtx,
			1,
			"location"
		}
	};
	for (int i = 0; i < 12; i++) {
		_syntaxRules.insert(std::make_pair(directives[i].str, directives[i]));
	}
}

void	Parser::_dupError(const std::string& str)
{
	std::stringstream	ss;

	ss << "\"" << str << "\"" << " is duplicate";
	throw SyntaxErrorException(ss.str());
}

void	Parser::_contextError(const std::string& str)
{
	std::stringstream	ss;

	ss << "\"" << str << "\"" << " directive is not allowed here";
	throw SyntaxErrorException(ss.str());
}

void	Parser::_argcError(const std::string& str)
{
	std::stringstream	ss;

	ss << "invalid number of arguments in "
		<< "\"" << str << "\"" << " directive";
	throw SyntaxErrorException(ss.str());
}

void	Parser::_parseContext(const Directive& directive)
{
	switch (directive.rules & Parser::kContext) {
		case Parser::kNoCtx :
			if (!_currConfig.empty())
				_contextError(directive.str);
			break;
		case Parser::kLocCtx :
			if (_currConfig.empty() || _currConfig.top().type != kLocation)
				_contextError(directive.str);
			break ;
		case Parser::kServCtx :
			if (_currConfig.empty() || _currConfig.top().type != kServer)
				_contextError(directive.str);
			break ;
	}
}

void	Parser::_parseDup(const Directive& directive)
{
	if ((directive.rules & Parser::kForbiddenDup) &&
		(_currConfig.top().isDefined[directive.type]))
			_dupError(directive.str);
}

void	Parser::_parseArgc(const Directive& directive, size_t argc)
{
	if (directive.rules & Parser::kArgcStrict) {
		if (directive.argc != argc)
			_argcError(directive.str);
	} else if (argc < directive.argc)
		_argcError(directive.str);
}

void	Parser::operator()(Lexer::token_queue& tokens)
{
	typedef Lexer::token_queue::iterator	it_t;
	typedef Directive						dir_t;

	it_t	directiveToken = tokens.begin();
	it_t	ctrlToken = std::find_if(directiveToken, tokens.end(), isNotWord);
	dir_t	directive = _syntaxRules.find(directiveToken->value)->second;
	size_t	argc = ctrlToken - directiveToken - 1;

	_parseContext(directive);
	_parseDup(directive);
	_parseArgc(directive, argc);
	// if (directive.rules & Parser::kIgnoreDup) {
	// 	for (size_t i = 0; i <= argc + 2; i++)
	// 		tokens.pop_front();
	// }
}

static bool	isNotWord(Lexer::Token tk)
{ return (tk.type != Lexer::Token::kWord); };

}	// namespace config

}	// namespace webserv