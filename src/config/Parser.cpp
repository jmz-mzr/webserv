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

#include "core/Socket.hpp"
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

Parser::Directive::Directive(it_t& first, it_t& last, DirectiveSyntax& s)
		: name(first->value)
		, ctrlToken(last)
		, syntax(s)
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

void	Parser::_errorHandler(const std::string& error_msg)
{ throw SyntaxErrorException(error_msg); }

void	Parser::_listenError(const std::string& err)
{
	_errorHandler(err + " in \"" + _currDirectivePtr->argv[0]
										+ "\" of the \"listen\" directive");
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
				_errorHandler("directive \"" + directive.name
											+ "\" is not terminated by \";\"");
			}
			break;
		case Parser::kBlock :
			if (directive.ctrlToken->type != Lexer::Token::kBlockStart) {
				_errorHandler("directive \"" + directive.name
												+ "\" has no opening \"{\"");
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
	int flag = (directive.syntax.rules & Parser::kContext);

	if (flag & Parser::kNoCtx && _configStack.empty())
		return ;
	else if (!_configStack.empty()) {
		if ((flag & Parser::kLocCtx && _configStack.top().type == kLocation)
			|| (flag & Parser::kServCtx && _configStack.top().type == kServer))
			return ;
	}
	_errorHandler("\"" + directive.syntax.str
						+ "\" directive is not allowed here");
}

/**
 * @brief Check if duplicate
 * 
 * @param directive 
 */
void	Parser::_parseDup(const Directive& directive)
{
	const Config::config_map& conf = _currConfig->getConfigs();

	if (_configStack.top().isDefined[directive.syntax.type]) {
		if (directive.syntax.type == kLocation) {
			if (conf.find(directive.argv[0]) == conf.end())
				return ;
		}
		_errorHandler("\"" + directive.syntax.str + "\" is duplicate");
	}
}

/**
 * @brief Check the argument count
 * 
 * @param directive 
 */
void	Parser::_parseArgc(const Directive& directive)
{
	if (((directive.syntax.rules & Parser::kArgcStrict)
			&& (directive.argv.size() != directive.syntax.argc))
		|| (directive.argv.size() < directive.syntax.argc))
		_errorHandler("invalid number of arguments in \""
						+ directive.syntax.str + "\" directive");
}

void	Parser::_parseDirective(it_t nameToken, it_t ctrlToken)
{
	std::map<std::string, DirectiveSyntax>::iterator syntaxIter;
	
	if ((syntaxIter = _grammar.find(nameToken->value)) == _grammar.end()) {
		_errorHandler("unknown directive \"" + nameToken->value + "\"");
	} else {
		Directive currDirective(nameToken, ctrlToken, syntaxIter->second);
		_currDirectivePtr = &currDirective;

		_parseType(currDirective);
		_parseContext(currDirective);
		if (currDirective.syntax.rules & Parser::kForbiddenDup)
			_parseDup(currDirective);
		_parseArgc(currDirective);
		if ((currDirective.syntax.rules & ~Parser::kIgnoreDup) //! MUST be checked first or could check empty stack
			|| !(_configStack.top().isDefined[currDirective.syntax.type])) {
			errno = 0;
			(this->*(currDirective.syntax.parseFn))(currDirective);
			_configStack.top().isDefined[currDirective.syntax.type] = true;
		}
	}
}

void	Parser::operator()(Lexer::token_queue& tokens)
{
	it_t		ctrlToken;

	while ((ctrlToken = std::find_if(tokens.begin(), tokens.end(),
										Lexer::isNotWord)) != tokens.end()) {
		if ((ctrlToken->type == Lexer::Token::kBlockEnd)
				&& (ctrlToken == tokens.begin())) {
			if ((_configStack.top().type == kServer)
					&& (_currConfig->getListens().empty())) {
				sockaddr_in	defaultAddr; 
				setSockAddr(defaultAddr, INADDR_ANY, 8000);
				_currConfig->addListenPair(defaultAddr);
			}
			_configStack.pop();
			_currConfig = &_configs.back();
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
	const std::string&	uri = currDirective.argv.back();
	long				errorCode;

	for (std::vector<std::string>::iterator it = currDirective.argv.begin();
			it != currDirective.argv.end() - 1;
			it++) {
		if (std::find_if(it->begin(), it->end(), &isnotdigit) != it->end())
			_errorHandler("invalid value \"" + (*it) + "\"");
		errorCode = strtol(it->c_str(), NULL, 10);
		if (errno)
			THROW_LOGIC("strtol(): " << strerror(errno));
		if ((errorCode < 300) || (errorCode > 599)) {
			_errorHandler("value \"" + (*it) + "\" must be between 300 and 599");
		}
		_currConfig->addErrorPage(static_cast<int>(errorCode), uri);
	}
}

void	Parser::_setMaxBodySize(Directive& currDirective)
{
	uint		shift;
	char*		unitPtr;
	const char*	strPtr = currDirective.argv[0].c_str();
	int64_t	size = strtoll(strPtr, &unitPtr, 10);

	if ((unitPtr == strPtr) || (strlen(unitPtr) > 1) || (size < 0))
		_errorHandler("\"client_max_body_size\" directive invalid value");
	if (errno)
		THROW_LOGIC("strtoll(): " << strerror(errno));
	switch (std::tolower(*unitPtr)) {
		case 'k': shift = 10; break;
		case 'm': shift = 20; break;
		case 'g': shift = 30; break;
		case 0: shift = 0; break;
		default:
			_errorHandler("\"client_max_body_size\" directive invalid value");
	}
	if (size > (LLONG_MAX >> shift))
		_errorHandler("\"client_max_body_size\" directive invalid value");
	size <<= shift;
	_currConfig->setMaxBodySize(size);
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
			_errorHandler("invalid method \"" + (*argIt) + "\"");
		_currConfig->addLimitExcept(*it);
	}
}

//TODO: URL parser
void	Parser::_setReturnPair(Directive& currDirective)
{
	(void)currDirective;
}

void	Parser::_setRoot(Directive& currDirective)
{
	_currConfig->setRoot(currDirective.argv[0]);
}

void	Parser::_setAutoIndex(Directive& currDirective)
{
	if (currDirective.argv[0] == "on")
		_currConfig->setAutoIndex(true);
	else if (currDirective.argv[0] == "off")
		_currConfig->setAutoIndex(false);
	else
		_errorHandler("invalid value \"" + currDirective.argv[0]
			+ "\" in \"autoindex\" directive, it must be \"on\" or \"off\"");
}

void	Parser::_setIndex(Directive& currDirective)
{
	_currConfig->setIndex(currDirective.argv[0]);
}

// TODO
void	Parser::_setFastCgiPass(Directive& currDirective)
{
	(void)currDirective;
}

// Each element of the hostname must be from 1 to 63 characters long
// and the entire hostname, including the dots, can be at most 253
// characters long. Valid characters for hostnames are ASCII(7)
// letters from a to z, the digits from 0 to 9, and the hyphen (-).
// A hostname may not start with a hyphen. A bad host name should throw
// "invalid host in "+=*&^%$!?<>" of the "listen" directive"
void	Parser::_parseHost(const std::string& str,
										std::list<sockaddr_in>& addrList)
{
	struct in_addr**	hostList;
	struct hostent*		hent = gethostbyname(str.c_str());
	in_port_t			port;

	if (hent == NULL)
		_listenError("host not found");
	hostList = reinterpret_cast<struct in_addr**>(hent->h_addr_list);
	port = addrList.back().sin_port;
	addrList.pop_back();
	for (int i = 0; hostList[i] != NULL; i++) {
		sockaddr_in	addr;
		initSockAddr(addr);
		addr.sin_addr.s_addr = hostList[i]->s_addr;
		addr.sin_port = port;
		addr.sin_family = AF_INET;
		addrList.push_back(addr);
	}
}

void	Parser::_parseAddress(const std::string& str,
										std::list<sockaddr_in>& addrList)
{
	if (str.empty())
		_listenError("no host");
	if (str == "*") {
		addrList.back().sin_addr.s_addr = INADDR_ANY;
	} else {
		in_addr_t tmp = inet_addr(str.c_str());
		if (tmp == unsigned(-1))
			_parseHost(str, addrList);
		else
			addrList.back().sin_addr.s_addr = tmp;
	}
}

int		Parser::_parsePort(const std::string& str,
									std::list<sockaddr_in>& addrList)
{
	sockaddr_in		addr;
	uint16_t		port;
	uint16_t		portMax = ((2 << 15) - 1);

	if (std::find_if(str.begin(), str.end(), &isnotdigit) != str.end())
	{
		setSockAddr(addr, INADDR_ANY, 80);
		addrList.push_back(addr);
		return (-1);
	} else {
		port = static_cast<uint16_t>(strtoul(str.c_str(), NULL, 10));
		if ((port == 0) || (port > portMax))
			_listenError("invalid port");
		setSockAddr(addr, INADDR_ANY, port);
		addrList.push_back(addr);
		return (0);
	}
}

void	Parser::_addListen(Directive& currDirective)
{
	std::list<sockaddr_in>	addrList;
	std::string 			input(currDirective.argv[0]);
	size_t					pos = input.find(':');

	if (pos != std::string::npos) {
		_parsePort(input.substr(pos + 1, input.size() - pos), addrList);
		_parseAddress(input.substr(0, pos), addrList);
	} else if (_parsePort(input, addrList) < 0)
		_parseAddress(input, addrList);
	for (std::list<sockaddr_in>::iterator	it = addrList.begin();
											it != addrList.end();
											it++) {
		if (_currConfig->addListenPair(*it) == false) {
			std::stringstream ss;
			ss << "a duplicate listen " << ft_inet_ntoa(it->sin_addr)
						<< ":" << ntohs(it->sin_port);
			_errorHandler(ss.str().c_str());
		}
	}
}

void	Parser::_addServerName(Directive& currDirective)
{
	_currConfig->addServerName(ft_str_tolower(currDirective.argv[0]));
}

// TODO: limit nested locations
void	Parser::_addLocation(Directive& currDirective)
{
	Config& locConf = _currConfig->addConfig(currDirective.argv[0], Config());

	_configStack.push(ConfigData(kLocation, locConf));
	_currConfig = &locConf;
}

void	Parser::_addServer(Directive& currDirective)
{
	(void)currDirective;
	_configs.push_back(Config());
	_configStack.push(ConfigData(kServer, _configs.back()));
	_currConfig = &_configs.back();
}

/******************************************************************************/
/*                            NON-MEMBER FUNCTIONS                            */
/******************************************************************************/

}	// namespace config

}	// namespace webserv