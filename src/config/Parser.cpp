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
	DirectiveSyntax	directives[12] = {
		{
			kErrorPage,
			kIgnoreDup | kServCtx,
			2,
			"error_page",
			&Parser::_addErrorPage
		},
		{
			kMaxBodySize,
			kForbiddenDup | kArgcStrict | kServCtx,
			1,
			"client_max_body_size",
			&Parser::_setMaxBodySize
		},
		{
			kLimitExcept,
			kIgnoreDup | kLocCtx,
			1,
			"limit_except",
			&Parser::_setLimitExcept
		},
		{
			kReturn,
			kIgnoreDup | kLocCtx,
			1,
			"return",
			&Parser::_setReturnPair
		},
		{
			kRoot,
			kForbiddenDup | kArgcStrict | kLocCtx,
			1,
			"root",
			&Parser::_setRoot
		},
		{
			kAutoindex,
			kForbiddenDup | kArgcStrict | kLocCtx,
			1,
			"autoindex",
			&Parser::_setAutoIndex
		},
		{
			kIndex,
			kIgnoreDup | kArgcStrict | kLocCtx,
			1,
			"index",
			&Parser::_setIndex
		},
		{
			kFastCgiPass,
			kForbiddenDup | kArgcStrict | kLocCtx,
			1,
			"fastcgi_pass",
			&Parser::_setFastCgiPass
		},
		{
			kListen,
			kAcceptDup | kArgcStrict | kServCtx,
			1,
			"listen",
			&Parser::_addListenPair
		},
		{
			kServerName,
			kAcceptDup | kServCtx,
			1,
			"server_name",
			&Parser::_addServerName
		},
		{
			kLocation,
			kForbiddenDup | kArgcStrict | kServCtx,
			1,
			"location",
			&Parser::_addLocation
		},
		{
			kServer,
			kAcceptDup | kArgcStrict | kNoCtx,
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
		: name(first->value)
{
	while (++first != last) {
		argv.push_back(first->value);
	}
}

Parser::ConfigData::ConfigData(Type t, Config& conf) : type(t), config(conf)
{ std::fill(isDefined, isDefined + 11, false); }

static bool	isNotWord(Lexer::Token tk)
{ return (tk.type != Lexer::Token::kWord); };

void	Parser::operator()(Lexer::token_queue& tokens)
{
	it_t ctrlToken;

	ctrlToken = std::find_if(tokens.begin(), tokens.end(), isNotWord);
	if (ctrlToken == tokens.end())
		return ;
	else if (ctrlToken->type == Lexer::Token::kBlockEnd) {
		_currConfig.pop();
		tokens.pop_front();
	} else {
		Directive	currDirective(tokens.begin(), ctrlToken);

		currDirective.syntax = _grammar.find(currDirective.name)->second;
		_parseContext(currDirective);
		_parseDup(currDirective);
		_parseArgc(currDirective);
		if (!(currDirective.syntax.rules & Parser::kIgnoreDup))
			(this->*(currDirective.syntax.parseFn))(currDirective);
		tokens.erase(tokens.begin(), ctrlToken + 1);
	}
}

void	Parser::_addErrorPage(Directive& currDirective)
{
	(void)currDirective;
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