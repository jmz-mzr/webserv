#include <arpa/inet.h>		// ntohs
#include <netdb.h>			// freeaddrinfo, gai_strerror, getaddrinfo
#include <netinet/in.h>		// in_port_t, sockaddr_in
#include <stddef.h>			// size_t
#include <stdint.h>			// int64_t, uint16_t
#include <sys/types.h>		// freeaddrinfo, gai_strerror, getaddrinfo
#include <sys/socket.h>		// freeaddrinfo, gai_strerror, getaddrinfo
#include <sys/stat.h>		// stat, struct stat

#include <cctype>			// isalnum, isalpha, isdigit, tolower
#include <cerrno>			// errno
#include <cstdlib>			// strtol, strtoll, strtoul
#include <cstring>			// memset, strerror, strlen

#include <algorithm>		// find_if, fill
#include <map>
#include <limits>			// numeric_limits
#include <list>
#include <set>
#include <sstream>
#include <string>
#include <utility>			// make_pair
#include <vector>

#include "config/Parser.hpp"
#include "utils/exceptions.hpp"
#include "utils/global_defs.hpp"
#include "utils/log.hpp"
#include "utils/utils.hpp"

namespace	webserv {

namespace	config {

static void	_errorHandler(const std::string& error_msg)
{ throw SyntaxErrorException(error_msg); }

static void	_listenError(const std::string& err, const std::string& arg)
{ _errorHandler(err + " in \"" + arg + "\" of the \"listen\" directive"); }

static bool isnotdigit(char c)
{ return (!bool(std::isdigit(c))); }

/******************************************************************************/
/*                         CONSTRUCTORS / DESTRUCTORS                         */
/******************************************************************************/

Parser::Parser()
{
	DirectiveSyntax	directives[Parser::kDirectiveNb] = {
		{
			kErrorPage,
			kDirective | kIgnoreDup | kServCtx | kLocCtx,
			2,
			"error_page",
			&Parser::_addErrorPage
		},
		{
			kMaxBodySize,
			kDirective | kForbiddenDup | kArgcStrict | kLocCtx | kServCtx,
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
			kDirective | kIgnoreDup | kLocCtx | kServCtx,
			1,
			"return",
			&Parser::_setReturnPair
		},
		{
			kRoot,
			kDirective | kForbiddenDup | kArgcStrict | kLocCtx | kServCtx,
			1,
			"root",
			&Parser::_setRoot
		},
		{
			kAlias,
			kDirective | kForbiddenDup | kArgcStrict | kLocCtx,
			1,
			"alias",
			&Parser::_setAlias
		},
		{
			kAutoindex,
			kDirective | kForbiddenDup | kArgcStrict | kLocCtx | kServCtx,
			1,
			"autoindex",
			&Parser::_setAutoIndex
		},
		{
			kIndex,
			kDirective | kAcceptDup | kArgcStrict | kLocCtx | kServCtx,
			1,
			"index",
			&Parser::_setIndex
		},
		{
			kCgiPass,
			kDirective | kForbiddenDup | kArgcStrict | kLocCtx,
			1,
			"cgi_pass",
			&Parser::_setCgiPass
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
	_methods.insert("head");
	_methods.insert("post");
	_methods.insert("put");
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

/**
 * @brief Check ending char
 * @param directive
 */
void	Parser::_parseType(const Directive& directive)
{
	switch (directive.syntax.rules & Parser::kType) {
		case Parser::kDirective :
			if (directive.ctrlToken->type != Lexer::Token::kDirectiveEnd) {
				_errorHandler("Directive \"" + directive.name
											+ "\" is not terminated by \";\"");
			}
			break;
		case Parser::kBlock :
			if (directive.ctrlToken->type != Lexer::Token::kBlockStart) {
				_errorHandler("Directive \"" + directive.name
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
	_errorHandler("Directive \"" + directive.syntax.str
						+ "\" is not allowed here");
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
		_errorHandler("Directive \"" + directive.syntax.str
				+ "\" is duplicated");
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
		_errorHandler("Invalid number of arguments in \""
						+ directive.syntax.str + "\" directive");
}

void	Parser::_parseDirective(it_t nameToken, it_t ctrlToken)
{
	std::map<std::string, DirectiveSyntax>::iterator syntaxIter;

	if ((syntaxIter = _grammar.find(nameToken->value)) == _grammar.end()) {
		_errorHandler("Unknown directive \"" + nameToken->value + "\"");
	} else {
		Directive currDirective(nameToken, ctrlToken, syntaxIter->second);
		_currDirectivePtr = &currDirective;

		_parseType(currDirective);
		_parseContext(currDirective);
		if (currDirective.syntax.rules & Parser::kForbiddenDup)
			_parseDup(currDirective);
		_parseArgc(currDirective);
		// TODO: MUST be checked first or could check empty stack !
		if ((currDirective.syntax.rules & ~Parser::kIgnoreDup)
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
			if (!_configStack.empty())
				_currConfig = &_configStack.top().config;
			tokens.pop_front();
		} else if (ctrlToken->type == Lexer::Token::kEOF) {
			tokens.pop_front();
		} else {
			_parseDirective(tokens.begin(), ctrlToken);
			tokens.erase(tokens.begin(), ctrlToken + 1);
		}
	}
}

void	Parser::_addErrorPage(Directive& currDirective)
{
	const std::string&	uri = currDirective.argv.back();
	long				errorCode;

	for (std::vector<std::string>::iterator it = currDirective.argv.begin();
			it != currDirective.argv.end() - 1;
			it++) {
		if (std::find_if(it->begin(), it->end(), &isnotdigit) != it->end())
			_errorHandler("Directive \"error_page\" invalid value \""
					+ *it + "\"");
		errorCode = std::strtol(it->c_str(), NULL, 10);
		if (errno)
			THROW_LOGIC("strtol(): " << std::strerror(errno));
		if ((errorCode < 300) || (errorCode > 599)) {
			_errorHandler("Directive \"error_page\" value \"" + *it
					+ "\" must be between 300 and 599");
		}
		_currConfig->addErrorPage(static_cast<int>(errorCode), uri);
	}
}

void	Parser::_setMaxBodySize(Directive& currDirective)
{
	uint		shift = 0;
	char*		unitPtr;
	const char*	strPtr = currDirective.argv[0].c_str();
	int64_t		size = std::strtoll(strPtr, &unitPtr, 10);

	if ((unitPtr == strPtr) || (std::strlen(unitPtr) > 1) || (size < 0))
		_errorHandler("Directive \"client_max_body_size\" invalid value \""
				+ currDirective.argv[0] + "\"");
	if (errno)
		THROW_LOGIC("strtoll(): " << std::strerror(errno));
	switch (std::tolower(*unitPtr)) {
		case 'k': shift = 10; break;
		case 'm': shift = 20; break;
		case 'g': shift = 30; break;
		case 0: shift = 0; break;
		default:
			_errorHandler("Directive \"client_max_body_size\" invalid value \""
					+ currDirective.argv[0] + "\"");
	}
	if (size > (std::numeric_limits<int64_t>::max() >> shift))
		_errorHandler("Directive \"client_max_body_size\" invalid value \""
				+ currDirective.argv[0] + "\"");
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
			_errorHandler("Invalid \"limit_except\" method \"" + *argIt + "\"");
		_currConfig->addLimitExcept(*it);
	}
}

void	Parser::_setReturnPair(Directive& currDirective)
{
	std::string		arg1(currDirective.argv[0]);
	size_t			pos = arg1.find(':');

	if (_currConfig->getReturnPair().first != -1)
		return ;
	if (pos != std::string::npos) {
		std::string	scheme = arg1.substr(0, pos);

		if ((scheme == "http" || scheme == "https")
				&& (arg1.compare(pos + 1, 2, "//") == 0))
			_currConfig->setReturnPair(std::make_pair(302, arg1));
		else
			_errorHandler("Invalid \"return\" directive \"" + arg1 + "\"");
	} else {
		long	errorCode;

		if (std::find_if(arg1.begin(), arg1.end(), &isnotdigit) != arg1.end())
			_errorHandler("Invalid \"return\" directive \"" + arg1 + "\"");
		errorCode = strtol(arg1.c_str(), NULL, 10);
		if (errno)
			THROW_LOGIC("strtol(): " << std::strerror(errno));
		if ((errorCode < 0) || (errorCode > 999))
			_errorHandler("Invalid \"return\" directive \"" + arg1 + "\"");
		if (currDirective.argv.size() > 1)
			_currConfig->setReturnPair(
							std::make_pair(errorCode, currDirective.argv[1]));
		else
			_currConfig->setReturnPair(std::make_pair(errorCode, ""));
	}
}

/**
 * @brief Check for duplicate alias/root directive (mutually exclusive)
 *
 * @param argType type of tested directive
 */
void	Parser::_duplicateCheck(const std::string& currDirStr,
								const size_t otherDirType,
								const std::string& otherDirStr)
{
	if (_configStack.top().isDefined[otherDirType])
		_errorHandler("Directive \"" + otherDirStr + "\" is duplicated, \"" +
					currDirStr + "\" directive was specified earlier");
}

void	Parser::_setRoot(Directive& currDirective)
{
	_duplicateCheck("root", kAlias, "alias");
	_currConfig->setRoot(currDirective.argv[0]);
}

void	Parser::_setAlias(Directive& currDirective)
{
	_duplicateCheck("alias", kRoot, "root");
	_currConfig->setAlias(currDirective.argv[0]);
}

void	Parser::_setAutoIndex(Directive& currDirective)
{
	if (currDirective.argv[0] == "on")
		_currConfig->setAutoIndex(true);
	else if (currDirective.argv[0] == "off")
		_currConfig->setAutoIndex(false);
	else
		_errorHandler("Invalid \"autoindex\" value \"" + currDirective.argv[0]
			+ "\", it must be \"on\" or \"off\"");
}

void	Parser::_setIndex(Directive& currDirective)
{
	_currConfig->setIndex(currDirective.argv[0]);
}

void	Parser::_setCgiPass(Directive& currDirective)
{
	std::string	path;
	struct stat	fileInfos;

	if (currDirective.argv[0][0] != '/') {
		path = XSTR(WEBSERV_ROOT);
		if (*(--(path.end())) != '/')
			path += "/";
	}
	path += currDirective.argv[0];
	errno = 0;
	if (stat(path.c_str(), &fileInfos) < 0) {
		THROW_FATAL("stat() error: " << path << ": " << std::strerror(errno));
	} else if (!S_ISREG(fileInfos.st_mode)/* || !(sb.st_mode & S_IXUSR)*/) {
		return (_errorHandler("\"" + path + "\" is not a regular file"));
	}
	_currConfig->setCgiPass(path);
}

static void	_addAddress(struct addrinfo* result,
											std::list<sockaddr_in>& addrList)
{
	in_port_t	port;

	port = addrList.back().sin_port;
	addrList.pop_back();
	for (struct addrinfo* currAddr = result; currAddr != NULL ; ) {
		sockaddr_in		addr;
		sockaddr_in*	tmp = reinterpret_cast<sockaddr_in*>(currAddr->ai_addr);

		initSockAddr(addr);
		addr.sin_addr.s_addr = tmp->sin_addr.s_addr;
		addr.sin_port = port;
		addr.sin_family = AF_INET;
		addrList.push_back(addr);
		currAddr = currAddr->ai_next;
	}
}

void	Parser::_parseHost(const std::string& str,
										std::list<sockaddr_in>& addrList)
{
	struct addrinfo		hints;
	struct addrinfo*	result;
	int					error;

	std::memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_socktype = SOCK_STREAM;
	error = getaddrinfo(str.c_str(), NULL, &hints, &result);
	if (error) {
		if (error == EAI_SYSTEM) {
			THROW_FATAL("getaddrinfo() error: " << std::strerror(errno));
		} else
			THROW_FATAL("getaddrinfo() error: " << gai_strerror(error));
	}
	if (result == NULL)
		_listenError("Host not found", _currDirectivePtr->argv[0]);
	_addAddress(result, addrList);
	freeaddrinfo(result);
}

void	Parser::_parseAddress(const std::string& str,
										std::list<sockaddr_in>& addrList)
{
	if (str.empty())
		_listenError("No host", _currDirectivePtr->argv[0]);
	if (str == "*") {
		addrList.back().sin_addr.s_addr = INADDR_ANY;
	} else {
		_parseHost(str, addrList);
	}
}

int		Parser::_parsePort(const std::string& str,
									std::list<sockaddr_in>& addrList)
{
	sockaddr_in		addr;
	unsigned long	port;
	unsigned long	portMax = ((2 << 15) - 1);

	if (std::find_if(str.begin(), str.end(), &isnotdigit) != str.end())
	{
		setSockAddr(addr, INADDR_ANY, 80);
		addrList.push_back(addr);
		return (-1);
	} else {
		errno = 0;
		port = std::strtoul(str.c_str(), NULL, 10);
		if (errno == ERANGE)
			return (-1);
		if ((port == 0) || (port > portMax))
			_listenError("invalid port", _currDirectivePtr->argv[0]);
		setSockAddr(addr, INADDR_ANY, static_cast<uint16_t>(port));
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
		if (_parsePort(input.substr(pos + 1, input.size() - pos), addrList) < 0)
			_listenError("Invalid port", &currDirective.argv[0][pos + 1]);
		_parseAddress(input.substr(0, pos), addrList);
	} else if (_parsePort(input, addrList) < 0)
		_parseAddress(input, addrList);
	for (std::list<sockaddr_in>::iterator	it = addrList.begin();
											it != addrList.end();
											it++) {
		if (_currConfig->addListenPair(*it) == false) {
			std::stringstream ss;
			ss << "Duplicated \"listen\" on \"" << ft_inet_ntoa(it->sin_addr)
						<< ":" << ntohs(it->sin_port);
			LOG_INFO(ss.str().c_str() << "\", ignored");
		}
	}
}

void	Parser::_addServerName(Directive& currDirective)
{
	if (currDirective.argv[0] == "\"\"")
		_currConfig->addServerName("");
	else
		_currConfig->addServerName(ft_str_tolower(currDirective.argv[0]));
}

void	Parser::_addLocation(Directive& currDirective)
{
	Config::LocationType	type;

	if (currDirective.argv[0][0] == '*' && currDirective.argv[0][1] == '.'
			&& std::isalnum(currDirective.argv[0][2])) {
		type = Config::kFile;
	} else
		type = Config::kPath;
	if ( ((_currConfig->getType() == Config::kPath) && (type == Config::kPath))
			|| (_currConfig->getType() == Config::kFile)
			|| (_configStack.size() > 2) )
		_errorHandler("Location \"" + currDirective.argv[0]
				+ "\" is too deeply nested");
	for (Config::config_map::const_iterator it = _currConfig->
			getConfigs().begin(); it != _currConfig->getConfigs().end(); ++it) {
		if (it->first == currDirective.argv[0]) {
			LOG_DEBUG("it->first = " << it->first);
			_errorHandler("Location \"" + currDirective.argv[0]
					+ "\" is duplicated");
		}
	}
	Config&	locConf = _currConfig->addConfig(currDirective.argv[0], Config());

	locConf.setType(type);
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
