#include "config/Config.hpp"

#include <map>
#include <string>
#include <vector>

namespace webserv {

namespace config {

void	Config::addErrorPage(const int status, const std::string& uri)
{ _errorPages.insert(std::make_pair(status, uri)); }

void	Config::setMaxBodySize(const long long size)
{ _maxBodySize = size; }

void	Config::setLimitExcept(uint flags)
{ _limitExcept = flags; }

void	Config::setReturnPair(std::pair<int, std::string> returnPair)
{ _return = returnPair; }

void	Config::setRoot(std::string path)
{ _root = path; }

void	Config::setAutoIndex(bool b)
{ _autoIndex = b; }

void	Config::setIndex(std::string path)
{ _index = path; }

void	Config::setFastCgiPass(std::string path)
{ _fastCgiPass = path; }

void	Config::addListenPair(const listen_pair& listenPair)
{ _listenPairs.push_back(listenPair); }

void	Config::addName(const std::string& name)
{ _serverNames.push_back(name); }

void	Config::addConfig(const Config& config)
{ _configs.push_back(config); }

}	// namespace config

}	// namespace webserv