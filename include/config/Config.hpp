#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <map>
#include <set>
#include <string>
#include <vector>

#include "utils/utils.hpp"
#include"utils/Logger.hpp"

namespace webserv {

namespace config {

	class Config {
	public:
		typedef std::pair<int, std::string>					return_pair;
		typedef std::set<std::string>						limit_except_set;
		typedef std::map<int, std::string>					error_pages_map;
		typedef std::map<std::string, Config>				config_map;

		class Address {
		public:
			Address(in_addr_t ip, in_port_t port)
					: _ip(ip)
					, _port(port)
					, _uniqueId(ip | (uint64_t(port) << 32))
			{ }
			friend bool	operator<(const Address& lhs, const Address& rhs)
			{ return (lhs._uniqueId < rhs._uniqueId); }

			in_addr_t	_ip;
			in_port_t	_port;
			uint64_t	_uniqueId;

		};

		Config();
		Config(const Config& src);
		~Config() {}

		void		addErrorPage(const int status, const std::string& uri);
		void		setMaxBodySize(const long long size);
		void		addLimitExcept(const std::string& method);
		void		setReturnPair(const return_pair& returnPair);
		void		setRoot(const std::string& path);
		void		setAutoIndex(bool b);
		void		setIndex(const std::string& path);
		void		setFastCgiPass(const std::string& path);
		bool		addListen(in_addr_t ip, in_port_t port);
		void		addServerName(const std::string& name);
		Config&		addConfig(const std::string& path, const Config& config);

		config_map&				getConfigs() { return (_configs); }
		std::set<std::string>&	getServerNames() { return (_serverNames); }

	private:
		error_pages_map				_errorPages;
		long long					_maxBodySize;
		limit_except_set			_limitExcept;
		return_pair					_return;
		std::string					_root;
		std::string					_index;
		bool						_autoIndex;
		std::string					_fastCgiPass;
		std::set<Address>			_listen;
		std::set<std::string>		_serverNames;
		config_map					_configs;

		Config&		operator=(const Config& rhs);

	};

}	// namespace config

}	// namespace webserv

#endif // CONFIG_HPP
