#include "config/Config.hpp"

#include <map>
#include <string>
#include <vector>

namespace webserv {

namespace config {

Config::Config()
		: _maxBodySize(0)
		, _limitExcept(0)
		, _autoIndex(false)
{ }

Config::Config(std::string p)
		: _maxBodySize(0)
		, _limitExcept(0)
		, _autoIndex(false)
		, _path(p)
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
		, _path(src._path)
{ }

void	Config::addErrorPage(const int status, const std::string& uri)
{ _errorPages.insert(std::make_pair(status, uri)); }

void	Config::setMaxBodySize(const long long size)
{ _maxBodySize = size; }

void	Config::setLimitExcept(uint flags)
{ _limitExcept = flags; }

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

void	Config::addName(const std::string& name)
{ _serverNames.push_back(name); }

void	Config::addConfig(const Config& config)
{ _configs.push_back(config); }

}	// namespace config

}	// namespace webserv