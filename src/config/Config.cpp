#include "config/Config.hpp"

#include <map>
#include <string>
#include <vector>

#include "utils/Logger.hpp"

namespace webserv {

Config::Config()
		: _lType(Config::kNone)
		, _maxBodySize(0)
		, _return(-1, "")
		, _root("html")
		, _alias("")
		, _index("index.html")
		, _autoIndex(false)
{ }

Config::Config(const Config& src)
		: _lType(src._lType)
		, _listens(src._listens)
		, _serverNames(src._serverNames)
		, _errorPages(src._errorPages)
		, _maxBodySize(src._maxBodySize)
		, _limitExcept(src._limitExcept)
		, _return(src._return)
		, _root(src._root)
		, _alias(src._alias)
		, _index(src._index)
		, _autoIndex(src._autoIndex)
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

void	Config::setReturnPair(const return_pair& returnPair)
{ _return = returnPair; }

void	Config::setRoot(const std::string& path)
{ _root = path; }

void	Config::setAlias(const std::string& path)
{ _alias = path; }

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
