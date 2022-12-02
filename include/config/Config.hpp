#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <map>
#include <string>
#include <vector>

#include "utils/utils.hpp"

namespace webserv {

namespace config {

	class Config {
		typedef std::pair<const int, std::string>			return_pair;
		typedef std::pair<const std::string, uint16_t>		listen_pair;
		typedef std::map<int, std::string>					error_pages_map;

		void	addErrorPage(const int status, const std::string& uri);
		void	setMaxBodySize(const long long size);
		void	setLimitExcept(uint flags);
		void	setReturnPair(std::pair<int, std::string> returnPair);
		void	setRoot(std::string path);
		void	setAutoIndex(bool b);
		void	setIndex(std::string path);
		void	setFastCgiPass(std::string path);
		void	addListenPair(const listen_pair& listenPair);
		void	addName(const std::string& name);
		void	addConfig(const Config& config);

		error_pages_map				_errorPages;
		long long					_maxBodySize;
		int							_limitExcept;
		return_pair					_return;
		std::string					_root;
		std::string					_index;
		bool						_autoIndex;
		std::string					_fastCgiPass;
		std::vector<listen_pair>	_listenPairs;
		std::vector<std::string>	_serverNames;
		std::vector<Config>			_configs;

	};

}	// namespace config

}	// namespace webserv

#endif	// CONFIG_HPP