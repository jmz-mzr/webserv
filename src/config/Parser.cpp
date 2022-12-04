#include "config/Parser.hpp"

#include <algorithm>
#include <cstdlib>
#include <sstream>
#include <string>

#include "utils/exceptions.hpp"
#include "utils/Logger.hpp"
#include "utils/utils.hpp"

namespace	webserv {

namespace	config {

Parser::Parser()
{
	DirectiveSyntax	directives[12] = {
		{
			kErrorPage,
			kDirective | kIgnoreDup | kServCtx,
			2,
			"error_page",
			&Parser::_addErrorPage
		},
		{
			kMaxBodySize,
			kDirective | kForbiddenDup | kArgcStrict | kServCtx,
			1,
			"client_max_body_size",
			&Parser::_setMaxBodySize
		},
		{
			kLimitExcept,
			kDirective | kIgnoreDup | kLocCtx,
			1,
			"limit_except",
			&Parser::_setLimitExcept
		},
		{
			kReturn,
			kDirective | kIgnoreDup | kLocCtx,
			1,
			"return",
			&Parser::_setReturnPair
		},
		{
			kRoot,
			kDirective | kForbiddenDup | kArgcStrict | kLocCtx,
			1,
			"root",
			&Parser::_setRoot
		},
		{
			kAutoindex,
			kDirective | kForbiddenDup | kArgcStrict | kLocCtx,
			1,
			"autoindex",
			&Parser::_setAutoIndex
		},
		{
			kIndex,
			kDirective | kIgnoreDup | kArgcStrict | kLocCtx,
			1,
			"index",
			&Parser::_setIndex
		},
		{
			kFastCgiPass,
			kDirective | kForbiddenDup | kArgcStrict | kLocCtx,
			1,
			"fastcgi_pass",
			&Parser::_setFastCgiPass
		},
		{
			kListen,
			kDirective | kAcceptDup | kArgcStrict | kServCtx,
			1,
			"listen",
			&Parser::_addListenPair
		},
		{
			kServerName,
			kDirective | kAcceptDup | kServCtx,
			1,
			"server_name",
			&Parser::_addServerName
		},
		{
			kLocation,
			kBlock | kForbiddenDup | kArgcStrict | kServCtx,
			1,
			"location",
			&Parser::_addLocation
		},
		{
			kServer,
			kBlock | kAcceptDup | kArgcStrict | kNoCtx,
			0,
			"server",
			&Parser::_addServer
		}
	};
	for (int i = 0; i < 12; i++) {
		_grammar.insert(std::make_pair(directives[i].str, directives[i]));
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

void	Parser::_parseType(const Directive& directive)
{
	switch (directive.syntax.rules & Parser::kType) {
		case Parser::kDirective :
			if (directive.ctrlToken != ";") {
				throw SyntaxErrorException("directive \"" + directive.name
											+ "\" is not terminated by \";\"");
			}
			break;
		case Parser::kBlock :
			if (directive.ctrlToken != "{") {
				throw SyntaxErrorException("directive \"" + directive.name
											+ "\" has no opening \"{\"");
			}
			break;
	}
}

void	Parser::_parseContext(const Directive& directive)
{
	switch (directive.syntax.rules & Parser::kContext) {
		case Parser::kNoCtx :
			if (!_currConfig.empty())
				_contextError(directive.syntax.str);
			break;
		case Parser::kLocCtx :
			if (_currConfig.empty() || _currConfig.top().type != kLocation)
				_contextError(directive.syntax.str);
			break ;
		case Parser::kServCtx :
			if (_currConfig.empty() || _currConfig.top().type != kServer)
				_contextError(directive.syntax.str);
			break ;
	}
}

void	Parser::_parseDup(const Directive& directive)
{
	if (directive.syntax.rules & Parser::kForbiddenDup) {
		if (directive.syntax.type == kLocation) {
			std::vector<Config>& confs = _currConfig.top().config.getConfigs();
			if (confs.empty() || directive.argv[0].empty())
				return ;
			for (std::vector<Config>::iterator it = confs.begin();
					it != confs.end();
					it++) {
				if (it->getPath() == directive.argv[0])
					_dupError(directive.syntax.str);
			}
		} else if (_currConfig.top().isDefined[directive.syntax.type])
			_dupError(directive.syntax.str);
	}
}

void	Parser::_parseArgc(const Directive& directive)
{
	if (directive.syntax.rules & Parser::kArgcStrict) {
		if (directive.argv.size() != directive.syntax.argc)
			_argcError(directive.syntax.str);
	} else if (directive.argv.size() < directive.syntax.argc)
		_argcError(directive.syntax.str);
}

Parser::Directive::Directive(it_t first, it_t last)
		: name(first->value), ctrlToken(last->value)
{
	while (++first != last) {
		argv.push_back(first->value);
	}
}

Parser::ConfigData::ConfigData(Type t, Config& conf) : type(t), config(conf)
{ std::fill(isDefined, isDefined + 11, false); }

static bool	isNotWord(Lexer::Token tk)
{ return (tk.type != Lexer::Token::kWord); };

void	Parser::_parseDirective(it_t nameToken, it_t ctrlToken)
{
	typedef std::map<std::string, DirectiveSyntax>::const_iterator	syntaxIt_t;

	Directive		currDirective(nameToken, ctrlToken);
	syntaxIt_t		it = _grammar.find(currDirective.name);
	
	if (it == _grammar.end()) {	
		throw SyntaxErrorException("unknown directive \""
									+ currDirective.name + "\"");
	}
	currDirective.syntax = it->second;
	_parseType(currDirective);
	_parseContext(currDirective);
	_parseDup(currDirective);
	_parseArgc(currDirective);
	if ((currDirective.syntax.rules & Parser::kAcceptDup) //! MUST be checked first or could check empty stack
		|| (_currConfig.top().isDefined[currDirective.syntax.type] == false))
	{
		(this->*(currDirective.syntax.parseFn))(currDirective);
		_currConfig.top().isDefined[currDirective.syntax.type] = true;
	}
}

void	Parser::operator()(Lexer::token_queue& tokens)
{
	it_t ctrlToken;

	while ((ctrlToken = std::find_if(tokens.begin(), tokens.end(), isNotWord))
			!= tokens.end()) {
		if ((ctrlToken->type == Lexer::Token::kBlockEnd)
				&& (ctrlToken == tokens.begin())) {
			_currConfig.pop();
			tokens.pop_front();
		} else {
			_parseDirective(tokens.begin(), ctrlToken);
			tokens.erase(tokens.begin(), ctrlToken + 1);
		}
	}
}

void	Parser::_addErrorPage(Directive& currDirective)
{
	int					errorCode;
	const std::string&	uri = currDirective.argv.back();

	for (std::vector<std::string>::iterator it = currDirective.argv.begin();
			it != currDirective.argv.end() - 1;
			it++) {
		errorCode = atoi(it->c_str());
		if ((errorCode < 300) || (errorCode > 599)) {
			throw SyntaxErrorException("value \"" + (*it)
										+ "\" must be between 300 and 599");
		}
		_currConfig.top().config.addErrorPage(errorCode, uri);
	}
	LOG_DEBUG("_addErrorPage");
}

void	Parser::_setMaxBodySize(Directive& currDirective)
{
	(void)currDirective;
	LOG_DEBUG("_setMaxBodySize");
}

void	Parser::_setLimitExcept(Directive& currDirective)
{
	(void)currDirective;
	LOG_DEBUG("_addLimitExcept");
}

void	Parser::_setReturnPair(Directive& currDirective)
{
	(void)currDirective;
	LOG_DEBUG("_setReturnPair");
}

void	Parser::_setRoot(Directive& currDirective)
{
	(void)currDirective;
	LOG_DEBUG("_setRoot");
}

void	Parser::_setAutoIndex(Directive& currDirective)
{
	(void)currDirective;
	LOG_DEBUG("_setAutoIndex");
}

void	Parser::_setIndex(Directive& currDirective)
{
	(void)currDirective;
	LOG_DEBUG("_setIndex");
}

void	Parser::_setFastCgiPass(Directive& currDirective)
{
	(void)currDirective;
	LOG_DEBUG("_setFastCgiPass");
}

void	Parser::_addListenPair(Directive& currDirective)
{
	(void)currDirective;
	LOG_DEBUG("_addListenPair");
}

void	Parser::_addServerName(Directive& currDirective)
{
	(void)currDirective;
	LOG_DEBUG("_addServerName");
}

void	Parser::_addLocation(Directive& currDirective)
{
	std::vector<Config>& _locationConfs = _currConfig.top().config.getConfigs();
	_locationConfs.push_back(Config(currDirective.argv[0]));
	_currConfig.push(ConfigData(kLocation, _locationConfs.back()));
	_currConfig.top().isDefined[kLocation] = true;
	LOG_DEBUG("_addLocation");
}

void	Parser::_addServer(Directive& currDirective)
{
	(void)currDirective;
	_configs.push_back(Config());
	_currConfig.push(ConfigData(kServer, _configs.back()));
	LOG_DEBUG("_addServer");
}

}	// namespace config

}	// namespace webserv