#include "config/Parser.hpp"

#include <algorithm>
#include <cctype>
#include <climits>
#include <cstdlib>
#include <limits>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <vector>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include "utils/exceptions.hpp"
#include "utils/Logger.hpp"
#include "utils/utils.hpp"

namespace	webserv {

namespace	config {

/******************************************************************************/
/*                         CONSTRUCTORS / DESTRUCTORS                         */
/******************************************************************************/

Parser::Parser()
{
	DirectiveSyntax	directives[Parser::kDirectiveNb] = {
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
			kDirective | kAcceptDup | kArgcStrict | kLocCtx,
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
			&Parser::_addListen
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
			kBlock | kForbiddenDup | kArgcStrict | kServCtx | kLocCtx,
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
	
	for (size_t i = 0; i < Parser::kDirectiveNb; i++) {
		_grammar.insert(std::make_pair(directives[i].str, directives[i]));
	}
	_methods.insert("get");
	_methods.insert("post");
	_methods.insert("delete");
}

Parser::Directive::Directive(it_t& first, it_t& last, DirectiveSyntax& syntax)
		: name(first->value)
		, ctrlToken(last)
		, syntax(syntax)
{
	while (++first != last) {
		argv.push_back(first->value);
	}
}

Parser::ConfigData::ConfigData(Type t, Config& conf) : type(t), config(conf)
{ std::fill(isDefined, isDefined + Parser::kDirectiveNb, false); }

/******************************************************************************/
/*                              MEMBER FUNCTIONS                              */
/******************************************************************************/

void	Parser::_dupError(const std::string& str)
{
	std::stringstream	ss;

	ss << "\"" << str << "\"" << " is duplicate";
	THROW_SYNTAX(ss.str());
}

void	Parser::_contextError(const std::string& str)
{
	std::stringstream	ss;

	ss << "\"" << str << "\"" << " directive is not allowed here";
	THROW_SYNTAX(ss.str());
}

void	Parser::_argcError(const std::string& str)
{
	std::stringstream	ss;

	ss << "invalid number of arguments in "
		<< "\"" << str << "\"" << " directive";
	THROW_SYNTAX(ss.str());
}

/**
 * @brief Check ending char
 * @param directive 
 */
void	Parser::_parseType(const Directive& directive)
{
	switch (directive.syntax.rules & Parser::kType) {
		case Parser::kDirective :
			if (directive.ctrlToken->type != Lexer::Token::kDirectiveEnd) {
				THROW_SYNTAX("directive \"" << directive.name
											<< "\" is not terminated by \";\"");
			}
			break;
		case Parser::kBlock :
			if (directive.ctrlToken->type != Lexer::Token::kBlockStart) {
				THROW_SYNTAX("directive \"" << directive.name
											<< "\" has no opening \"{\"");
			}
			break;
	}
}

/**
 * @brief Check the containing block
 * 
 * @param directive 
 */
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

/**
 * @brief Check if duplicate
 * 
 * @param directive 
 */
void	Parser::_parseDup(const Directive& directive)
{
	const Config::config_map& conf = _currConfig.top().config.getConfigs();

	if (_currConfig.top().isDefined[directive.syntax.type]) {
		if (directive.syntax.type == kLocation) {
			if (conf.find(directive.argv[0]) != conf.end())
				_dupError(directive.syntax.str);
		} else {
			_dupError(directive.syntax.str);
		}
	}
}

/**
 * @brief Check the argument count
 * 
 * @param directive 
 */
void	Parser::_parseArgc(const Directive& directive)
{
	if (directive.syntax.rules & Parser::kArgcStrict) {
		if (directive.argv.size() != directive.syntax.argc)
			_argcError(directive.syntax.str);
	} else if (directive.argv.size() < directive.syntax.argc)
		_argcError(directive.syntax.str);
}

void	Parser::_parseDirective(it_t nameToken, it_t ctrlToken)
{
	std::map<std::string, DirectiveSyntax>::iterator syntaxIter;
	
	if ((syntaxIter = _grammar.find(nameToken->value)) == _grammar.end()) {
		THROW_SYNTAX("unknown directive \"" << nameToken->value << "\"");
	} else {
		Directive currDirective(nameToken, ctrlToken, syntaxIter->second);

		_parseType(currDirective);
		_parseContext(currDirective);
		if (currDirective.syntax.rules & Parser::kForbiddenDup)
			_parseDup(currDirective);
		_parseArgc(currDirective);
		if ((currDirective.syntax.rules & ~Parser::kIgnoreDup) //! MUST be checked first or could check empty stack
			|| !(_currConfig.top().isDefined[currDirective.syntax.type])) {
			errno = 0;
			(this->*(currDirective.syntax.parseFn))(currDirective);
			_currConfig.top().isDefined[currDirective.syntax.type] = true;
		}
	}
}

void	Parser::operator()(Lexer::token_queue& tokens)
{
	it_t	ctrlToken;

	while ((ctrlToken = std::find_if(tokens.begin(), tokens.end(),
										Lexer::isNotWord)) != tokens.end()) {
		if ((ctrlToken->type == Lexer::Token::kBlockEnd)
				&& (ctrlToken == tokens.begin())) {
			if ((_currConfig.top().type == kServer)
					&& (_currConfig.top().config.getListens().empty())) {
				sockaddr_in	addr;
				addr.sin_addr.s_addr = INADDR_ANY;
				addr.sin_port = htons(8000);
				_currConfig.top().config.addListen(addr);
			}
			_currConfig.pop();
			tokens.pop_front();
		} else if (ctrlToken->type == Lexer::Token::kEOF) {
			tokens.pop_front();
		} else {
			_parseDirective(tokens.begin(), ctrlToken);
			tokens.erase(tokens.begin(), ctrlToken + 1);
		}
	}
}

static bool isnotdigit(char c)
{ return (!bool(std::isdigit(c))); }

void	Parser::_addErrorPage(Directive& currDirective)
{
	Config& 			config = _currConfig.top().config;
	const std::string&	uri = currDirective.argv.back();
	long				errorCode;

	for (std::vector<std::string>::iterator it = currDirective.argv.begin();
			it != currDirective.argv.end() - 1;
			it++) {
		if (std::find_if(it->begin(), it->end(), &isnotdigit) != it->end())
			THROW_SYNTAX("invalid value \"" << (*it) << "\"");
		errorCode = strtol(it->c_str(), NULL, 10);
		if (errno)
			THROW_FATAL("strtol(): " << strerror(errno));
		if ((errorCode < 300) || (errorCode > 599)) {
			THROW_SYNTAX("value \"" << (*it) << "\" must be between 300 and 599");
		}
		config.addErrorPage(errorCode, uri);
	}
	LOG_DEBUG("_addErrorPage");
}

void	Parser::_setMaxBodySize(Directive& currDirective)
{
	uint		shift;
	char*		unitPtr;
	const char*	strPtr = currDirective.argv[0].c_str();
	long long	size = strtoll(strPtr, &unitPtr, 10);

	if ((unitPtr == strPtr) || (strlen(unitPtr) > 1) || (size < 0))
		THROW_SYNTAX("\"client_max_body_size\" directive invalid value");
	if (errno)
		THROW_FATAL("strtoll(): " << strerror(errno));
	switch (std::tolower(*unitPtr)) {
		case 'k': shift = 10; break;
		case 'm': shift = 20; break;
		case 'g': shift = 30; break;
		case 0: shift = 0; break;
		default:
			THROW_SYNTAX("\"client_max_body_size\" directive invalid value");
	}
	if (size > (LLONG_MAX >> shift))
		THROW_SYNTAX("\"client_max_body_size\" directive invalid value");
	size <<= shift;
	_currConfig.top().config.setMaxBodySize(size);
	LOG_DEBUG("_setMaxBodySize");
}

void	Parser::_setLimitExcept(Directive& currDirective)
{
	typedef std::vector<std::string>::const_iterator	argIter_t;
	typedef std::set<std::string>::const_iterator		methodIter_t;

	for (argIter_t argIt = currDirective.argv.begin();
			argIt != currDirective.argv.end();
			argIt++) {
		methodIter_t it = _methods.find(ft_str_tolower(*argIt));
		if (it == _methods.end())
			THROW_SYNTAX("invalid method \"" << (*argIt) << "\"");
		_currConfig.top().config.addLimitExcept(*it);
	}
	LOG_DEBUG("_addLimitExcept");
}

//TODO: URL parser
void	Parser::_setReturnPair(Directive& currDirective)
{
	(void)currDirective;
	LOG_DEBUG("_setReturnPair");
}

void	Parser::_setRoot(Directive& currDirective)
{
	_currConfig.top().config.setRoot(currDirective.argv[0]);
	LOG_DEBUG("_setRoot");
}

void	Parser::_setAutoIndex(Directive& currDirective)
{
	if (currDirective.argv[0] == "on")
		_currConfig.top().config.setAutoIndex(true);
	else if (currDirective.argv[0] == "off")
		_currConfig.top().config.setAutoIndex(false);
	else
		THROW_SYNTAX("invalid value \"" << currDirective.argv[0]
			<< "\" in \"autoindex\" directive, it must be \"on\" or \"off\"");
	LOG_DEBUG("_setAutoIndex");
}

void	Parser::_setIndex(Directive& currDirective)
{
	_currConfig.top().config.setIndex(currDirective.argv[0]);
	LOG_DEBUG("_setIndex");
}

//TODO
void	Parser::_setFastCgiPass(Directive& currDirective)
{
	(void)currDirective;
	LOG_DEBUG("_setFastCgiPass");
}

static void	_parsePort(const std::vector<std::string>& parts, in_port_t& port)
{
	char*			endptr;
	unsigned long	tmp;
	uint			portMax = ((2 << 15) - 1);

	errno = 0;
	for (int i = 0; i < 2; i++) {
		if (parts[i].empty())
			continue ;
		if (port != 0)
			return ;
		if (std::find_if(parts[i].begin(), parts[i].end(), &isnotdigit)
											== parts[i].end()) {
			tmp = strtol(parts[i].c_str(), &endptr, 10);
			if ((tmp == 0) || (tmp > portMax))
				THROW_LOGIC("invalid port in \"" << parts[i]
								<< "\" of the \"listen\" directive");
			if (errno)
				THROW_FATAL("strtol(): " << strerror(errno));
			port = htons(tmp);
		}
	}
}

bool	Parser::_resolveHost(const std::string& str, sockaddr_in& addr,
									Config& conf)
{
	struct in_addr**	addr_list;
	struct hostent*		hent = gethostbyname(str.c_str());

	if (hent == NULL)
		THROW_LOGIC("host not found in \"" << str
									<< "\" of the \"listen\" directive");
	addr_list = reinterpret_cast<struct in_addr**>(hent->h_addr_list);
	for (int i = 0; addr_list[i] != NULL; i++) {
		addr.sin_addr.s_addr = addr_list[i]->s_addr;
		if (conf.addListen(addr) == true) {
			return (true);
		} else {
			std::stringstream ss;
			ss << "a duplicate listen " << inet_ntoa(*addr_list[i])
				<< ":" << addr.sin_port;
			THROW_SYNTAX(ss.str());
		}
	}
	return (false);
}

bool	Parser::_parseListenUnit(const std::vector<std::string>& part,
									sockaddr_in& addr)
{
	_parsePort(part, addr.sin_port);
	for (int i = 0; i < 2; i++) {
		if (addr.sin_addr.s_addr != 0)
			return (false);
		if (part[i] == "*")
			addr.sin_addr.s_addr = INADDR_ANY;
		else if ((ft_inet_aton(part[i].c_str(), &addr.sin_addr) == 0)
				&& _resolveHost(part[i], addr, _currConfig.top().config))
			return (true);
	}
	return (false);
}

void	Parser::_addListen(Directive& currDirective)
{
	sockaddr_in					addr;
	std::vector<std::string> 	part(2);
	std::istringstream 			input(currDirective.argv[0]);

	memset(&addr, 0, sizeof(addr));
	std::getline(input, part[0], ':');
	std::getline(input, part[1]);
	if (_parseListenUnit(part, addr))
		return ;
	if (!addr.sin_port && addr.sin_addr.s_addr)
		addr.sin_port = htons(80);
	else if (!(addr.sin_addr.s_addr) && addr.sin_port)
		addr.sin_addr.s_addr = INADDR_ANY;
	if (_currConfig.top().config.addListen(addr) == false) {
		std::stringstream ss;
		ss << "a duplicate listen " << ft_inet_ntoa(addr.sin_addr) << ":"
			<< ntohs(addr.sin_port);
		THROW_SYNTAX(ss.str());
	}
	LOG_DEBUG("_addListen");
}

void	Parser::_addServerName(Directive& currDirective)
{
	Config& config = _currConfig.top().config;

	config.addServerName(ft_str_tolower(currDirective.argv[0]));
	LOG_DEBUG("_addServerName");
}

//TODO: limit nested locations
void	Parser::_addLocation(Directive& currDirective)
{
	Config& serverConf = _currConfig.top().config;
	Config& locConf = serverConf.addConfig(currDirective.argv[0], Config());

	_currConfig.push(ConfigData(kLocation, locConf));
	LOG_DEBUG("_addLocation");
}

void	Parser::_addServer(Directive& currDirective)
{
	(void)currDirective;
	_configs.push_back(Config());
	_currConfig.push(ConfigData(kServer, _configs.back()));
	LOG_DEBUG("_addServer");
}

/******************************************************************************/
/*                            NON-MEMBER FUNCTIONS                            */
/******************************************************************************/

}	// namespace config

}	// namespace webserv