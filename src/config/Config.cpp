#include <arpa/inet.h>	// ntohs

#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <utility>			// make_pair

#include "config/Config.hpp"
#include "utils/log.hpp"

namespace webserv {

Config::Config()
		: _lType(Config::kNone)
		, _maxBodySize(-1)
		, _returnPair(-1, "")
		, _autoIndex(false)
		, _hideLimitRule(false)
		, _hideDirectory(false)
{ }

Config::Config(const Config& src)
		: _lType(src._lType)
		, _listens(src._listens)
		, _serverNames(src._serverNames)
		, _errorPages(src._errorPages)
		, _maxBodySize(src._maxBodySize)
		, _limitExcept(src._limitExcept)
		, _ignoreExcept(src._ignoreExcept)
		, _returnPair(src._returnPair)
		, _root(src._root)
		, _alias(src._alias)
		, _index(src._index)
		, _autoIndex(src._autoIndex)
		, _hideLimitRule(src._hideLimitRule)
		, _hideDirectory(src._hideDirectory)
		, _cgiPass(src._cgiPass)
		, _configs(src._configs)
{ }

void	Config::setType(const LocationType type)
{ _lType = type; }

void	Config::addErrorPage(const int status, const std::string& uri)
{ _errorPages.insert(std::make_pair(status, uri)); }

void	Config::setMaxBodySize(const int64_t size)
{ _maxBodySize = size; }

void	Config::addLimitExcept(const std::string& method)
{ _limitExcept.insert(method); }

void	Config::addIgnoreExcept(const std::string& method)
{ _ignoreExcept.insert(method); }

void	Config::setReturnPair(const return_pair& returnPair)
{ _returnPair = returnPair; }

void	Config::setRoot(const std::string& path)
{ _root = path; }

void	Config::setAlias(const std::string& path)
{ _alias = path; }

void	Config::setHideDirectory(bool b)
{ _hideDirectory = b; }

void	Config::setHideLimitRule(bool b)
{ _hideLimitRule = b; }

void	Config::setAutoIndex(bool b)
{ _autoIndex = b; }

void	Config::setIndex(const std::string& path)
{ _index = path; }

void	Config::setCgiPass(const std::string& path)
{ _cgiPass = path; }

bool	Config::addListenPair(const sockaddr_in& addr)
{
	LOG_DEBUG(ft_inet_ntoa(addr.sin_addr) << ":" << ntohs(addr.sin_port));
	return (_listens.insert(addr).second);
}

void	Config::addServerName(const std::string& name)
{ _serverNames.insert(name); }

Config&	Config::addConfig(const std::string& path, const Config& config)
{ return (_configs.insert(std::make_pair(path, config)).first->second); }

/******************************************************************************/
/*                            NON-MEMBER FUNCTIONS                            */
/******************************************************************************/

std::ostream&	operator<<(std::ostream& os, const Config& conf)
{
	os << "listens:" << conf.getListens().size() << " "
		<< "server_name:" << conf.getServerNames().size() << " "
		<< "error_pages:" << conf.getErrorPages().size() << " "
		<< "max_body_size:" << conf.getMaxBodySize() << " "
		<< "limit_except:" << conf.getLimitExcept().size() << " "
		<< "return:" << conf.getReturnPair().first << "/"
					<< conf.getReturnPair().second << " "
		<< "root:" << conf.getRoot() << " "
		<< "alias:" << conf.getAlias() << " "
		<< "autoindex:" << (conf.isAutoIndex() ? "true" : "false") << " "
		<< "index:" << conf.getIndex() << " "
		<< "cgi:" << conf.getCgiPass() << " "
		<< "locations:" << conf.getConfigs().size() << std::endl;
	return (os);
}

}	// namespace webserv
