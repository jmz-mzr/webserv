#ifndef CONFIG_CONFIG_HPP
# define CONFIG_CONFIG_HPP

# include <stdint.h>		// int64_t
# include <netinet/in.h>	// sockaddr_in

# include <map>
# include <iostream>
# include <set>
# include <string>
# include <utility>			// pair
# include <vector>

# include "core/Socket.hpp"
# include "utils/log.hpp"
# include "utils/utils.hpp"

namespace webserv {

	class	Config {
	public:
		typedef std::pair<int, std::string>					return_pair;
		typedef std::set<std::string, strcmp_icase>			limit_except_set;
		typedef std::set<std::string, strcmp_icase>			ignore_except_set;
		typedef std::map<int, std::string>					error_page_map;
		typedef std::map<std::string, Config>				config_map;
		typedef std::set<std::string, strcmp_icase>			hostname_set;
		typedef std::set<sockaddr_in, listen_compare>		listen_set;

		enum LocationType { kNone, kPath, kFile };

		Config();
		Config(const Config& src);
		~Config() {}

		void		setType(const LocationType type);
		void		addErrorPage(const int status, const std::string& uri);
		void		setMaxBodySize(const int64_t size);
		void		addLimitExcept(const std::string& method);
		void		addIgnoreExcept(const std::string& method);
		void		setReturnPair(const return_pair& returnPair);
		void		setRoot(const std::string& path);
		void		setAlias(const std::string& path);
		void		setHideDirectory(bool b);
		void		setHideLimitRule(bool b);
		void		setAutoIndex(bool b);
		void		setIndex(const std::string& path);
		void		setCgiPass(const std::string& path);
		bool		addListenPair(const sockaddr_in& addr);
		void		addServerName(const std::string& name);
		Config&		addConfig(const std::string& path, const Config& config);

		const LocationType&			getType() const
												{ return (_lType); }
		const listen_set&			getListens() const
												{ return (_listens); }
		const hostname_set&			getServerNames() const
												{ return (_serverNames); }
		const error_page_map&		getErrorPages() const
												{ return (_errorPages); }
		const int64_t&				getMaxBodySize() const
												{ return (_maxBodySize); }
		const limit_except_set&		getLimitExcept() const
												{ return (_limitExcept); }
		const ignore_except_set&	getIgnoreExcept() const
												{ return (_ignoreExcept); }
		const return_pair&			getReturnPair() const
												{ return (_returnPair); }
		const std::string&			getRoot() const
												{ return (_root); }
		const std::string&			getAlias() const
												{ return (_alias); }
		const std::string&			getIndex() const
												{ return (_index); }
		int							isAutoIndex() const
												{ return (_autoIndex); }
		int							hideLimitRule() const
												{ return (_hideLimitRule); }
		int							hideDirectory() const
												{ return (_hideDirectory); }
		const std::string&			getCgiPass() const
												{ return (_cgiPass); }
		const config_map&			getConfigs() const
												{ return (_configs); }

		friend std::ostream&	operator<<(std::ostream&, const Config&);
	private:
		Config&		operator=(const Config& rhs);

		LocationType				_lType;
		listen_set					_listens;
		hostname_set				_serverNames;
		error_page_map				_errorPages;
		int64_t						_maxBodySize;
		limit_except_set			_limitExcept;
		ignore_except_set			_ignoreExcept;
		return_pair					_returnPair;
		std::string					_root;
		std::string					_alias;
		std::string					_index;
		int							_autoIndex;
		int							_hideLimitRule;
		int							_hideDirectory;
		std::string					_cgiPass;
		config_map					_configs;
	};

}	// namespace webserv

#endif	// CONFIG_CONFIG_HPP
