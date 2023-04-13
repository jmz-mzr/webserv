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
		bool						isAutoIndex() const
												{ return (_autoIndex); }
		bool						hideLimitRule() const
												{ return (_hideLimitRule); }
		bool						hideDirectory() const
												{ return (_hideDirectory); }
		const std::string&			getCgiPass() const
												{ return (_cgiPass); }
		const config_map&			getConfigs() const
												{ return (_configs); }

		friend std::ostream&	operator<<(std::ostream&, const Config&);
	private:
		LocationType				_lType;
		listen_set					_listens;
		hostname_set				_serverNames;
		error_page_map				_errorPages;
		int64_t						_maxBodySize;
		limit_except_set			_limitExcept;
		ignore_except_set			_ignoreExcept;
		return_pair					_returnPair;

		// 1) Can only be defined once, and if another definition line
		// appears, it must throw an exception (like '"root" directive is
		// duplicate in /usr/local/etc/nginx/nginx.conf:117')
		// 2) If it was not defined it must not stay empty, and like NGINX
		// it must be set to the default _root: "html"
		// 3) Cannot be defined if "alias" was already specified in a location,
		// otherwise it must throw an exception (like '"root" directive is
		// duplicate, "alias" directive was specified earlier in
		// /usr/local/etc/nginx/nginx.conf:68)
		std::string					_root;

		// 1) Can only be defined once, and if another definition line
		// appears, it must throw an exception (like '"alias" directive is
		// duplicate in /usr/local/etc/nginx/nginx.conf:68')
		// 2) Cannot be defined if "root" was already specified in a location,
		// otherwise it must throw an exception (like '"alias" directive is
		// duplicate, "root" directive was specified earlier in
		// /usr/local/etc/nginx/nginx.conf:68)
		std::string					_alias;

		// 1) For the sake of simplicity, accept only one default file
		// to answer if the request is a directory (unlike NGINX's index)
		// 2) Like NGINX's, it always has a default _index: "index.html",
		// already set when creating a Location class
		// 3) Can be defined multiple times, and for the sake of simplicity,
		// if another definition line appears, it replaces the previous one
		std::string					_index;

		// 1) Can only be defined once, and if another
		// definition line appears, it must throw an exception
		// (like '"autoindex" directive is duplicate in
		// /usr/local/etc/nginx/nginx.conf:37')
		bool						_autoIndex;

		// 1) Can only be defined once, and if another
		// definition line appears, it must throw an exception
		// (like '"hide_limit_rule/hide_directory" directive is duplicate in
		// /usr/local/etc/nginx/nginx.conf:37')
		bool						_hideLimitRule;
		bool						_hideDirectory;

		// 1) Can only be defined once, and if another definition line
		// appears, it must throw an exception (like '"cgi_pass" directive
		// is duplicate in /usr/local/etc/nginx/nginx.conf:109')
		// 2) It must be either a valid IP address, or a valid hostname (no special
		// characters, and can be translated, case-insensitively to an IP address
		// with the hosts file), and a port, otherwise it throw an exception (like
		// 'no port in upstream "localhost" in /usr/local/etc/nginx/nginx.conf:114',
		// 'invalid host in upstream "http:/localhost:80" in /.../nginx.conf:133',
		// 'invalid port in upstream "loCalhOst:80000" in /usr/.../nginx.conf:133',
		// 'host not found in upstream "127.0.0.1000" in /usr/.../nginx.conf:133',
		// 'host not found in upstream "localhosttt" in /usr/.../nginx.conf:133')
		std::string					_cgiPass;

		// TO DO: 1) The locations are case-insensitive, so they must
		// be recorded in lowercase with str_tolower
		// 2) A given location can only appear once in a server
		// otherwise it must throw an exception (like 'duplicate
		// location "/abc" in /usr/local/etc/nginx/nginx.conf:91')
		// 3) The location cannot be empty, otherwise it must
		// throw an exception (like 'invalid number of arguments in
		// "location" directive in /usr/local/etc/nginx/nginx.conf:95')
		// 4) It has at least an empty default location, with parameters
		// inherited from the ServerConfig
		// 5) Extension locations must follow the form "\*\.(alnum|$|.|_|-)+"
		// (star, dot, and then any NON-EMPTY (total size > 2) combination of
		// alphanum/dollar/dot/underscore/dash)
		// 6) For the sake of simplicity, only allow nested locations if they
		// are extension locations in normal locations
		config_map					_configs;

		Config&						operator=(const Config& rhs);
	};

}	// namespace webserv

#endif	// CONFIG_CONFIG_HPP
