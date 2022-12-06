#include "config/Config.hpp"

#include <map>
#include <string>
#include <vector>

namespace webserv {

namespace config {

Config::Config()
		: _maxBodySize(0)
		, _autoIndex(false)
{ }

Config::Config(const Config& src)
		: _errorPages(src._errorPages)
		, _maxBodySize(src._maxBodySize)
		, _limitExcept(src._limitExcept)
		, _return(src._return)
		, _root(src._root)
		, _index(src._index)
		, _autoIndex(src._autoIndex)
		, _fastCgiPass(src._fastCgiPass)
		, _listenPairs(src._listenPairs)
		, _serverNames(src._serverNames)
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

void	Config::addListenPair(const listen_pair& listenPair)
{ _listenPairs.push_back(listenPair); }

void	Config::addServerName(const std::string& name)
{ _serverNames.insert(name); }

Config&	Config::addConfig(const std::string& path, const Config& config)
{ return (_configs.insert(std::make_pair(path, config)).first->second); }

}	// namespace config

}	// namespace webserv