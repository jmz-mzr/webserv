#include "config/Config.hpp"

#include <map>
#include <string>
#include <vector>

#include "utils/Logger.hpp"

namespace webserv {

Config::Config()
		: _maxBodySize(0)
		, _root("html")
		, _index("index.html")
		, _autoIndex(false)
{ }

Config::Config(const Config& src)
		: _listens(src._listens)
		, _serverNames(src._serverNames)
		, _errorPages(src._errorPages)
		, _maxBodySize(src._maxBodySize)
		, _limitExcept(src._limitExcept)
		, _return(src._return)
		, _root(src._root)
		, _index(src._index)
		, _autoIndex(src._autoIndex)
		, _fastCgiPass(src._fastCgiPass)
		, _configs(src._configs)
{ }

void	Config::addErrorPage(const int status, const std::string& uri)
{ _errorPages.insert(std::make_pair(status, uri)); }

void	Config::setMaxBodySize(const long long size)
{ _maxBodySize = size; }

void	Config::addLimitExcept(const std::string& method)
{ _limitExcept.insert(method); }

void	Config::setReturnPair(const return_pair& returnPair)
{ _return = returnPair; }

void	Config::setRoot(const std::string& path)
{ _root = path; }

void	Config::setAutoIndex(bool b)
{ _autoIndex = b; }

void	Config::setIndex(const std::string& path)
{ _index = path; }

void	Config::setFastCgiPass(const std::string& path)
{ _fastCgiPass = path; }

bool	Config::addListen(in_addr ip, in_port_t port)
{
	LOG_DEBUG(inet_ntoa(ip) << ":" << port);
	return (_listens.insert(Address(ip.s_addr, port)).second); 
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
		<< "autoindex:" << (conf.isAutoIndex() ? "true" : "false") << " "
		<< "index:" << conf.getIndex() << " "
		<< "fastcgi:" << conf.getFastCgiPass() << " "
		<< "locations:" << conf.getConfigs().size() << std::endl;
	return (os);
}

}	// namespace webserv