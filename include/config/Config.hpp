#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <map>
#include <string>
#include <vector>

#include "utils/utils.hpp"

namespace webserv {

namespace config {

	class Config {
	public:
		typedef std::pair<int, std::string>					return_pair;
		typedef std::pair<std::string, uint16_t>			listen_pair;
		typedef std::map<int, std::string>					error_pages_map;

		Config();
		Config(std::string p);
		Config(const Config& src);
		~Config() {}

		void	addErrorPage(const int status, const std::string& uri);
		void	setMaxBodySize(const long long size);
		void	setLimitExcept(uint flags);
		void	setReturnPair(const return_pair& returnPair);
		void	setRoot(const std::string& path);
		void	setAutoIndex(bool b);
		void	setIndex(const std::string& path);
		void	setFastCgiPass(const std::string& path);
		void	addListenPair(const listen_pair& listenPair);
		void	addName(const std::string& name);
		void	addConfig(const Config& config);

		std::vector<Config>&	getConfigs() { return (_configs); }
		std::string&			getPath() { return (_path); }

	private:
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
		std::string					_path;

		Config&		operator=(const Config& rhs);

	};

}	// namespace config

}	// namespace webserv

#endif	// CONFIG_HPP