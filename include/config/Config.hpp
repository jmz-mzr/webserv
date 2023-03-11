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
		void		setReturnPair(const return_pair& returnPair);
		void		setRoot(const std::string& path);
		void		setAlias(const std::string& path);
		void		setAutoIndex(bool b);
		void		setIndex(const std::string& path);
		void		setCgiPass(const std::string& path);
		bool		addListenPair(const sockaddr_in& addr);
		void		addServerName(const std::string& name);
		Config&		addConfig(const std::string& path, const Config& config);

		const LocationType&		getType() const { return (_lType); }
		const listen_set&		getListens() const { return (_listens); }
		const hostname_set&		getServerNames() const { return (_serverNames);}
		const error_page_map&	getErrorPages() const { return (_errorPages); }
		const int64_t&			getMaxBodySize() const { return (_maxBodySize);}
		const limit_except_set&	getLimitExcept() const { return (_limitExcept);}
		const return_pair&		getReturnPair() const { return (_returnPair); }
		const std::string&		getRoot() const { return (_root); }
		const std::string&		getAlias() const { return (_alias); }
		bool					isAutoIndex() const { return (_autoIndex); }
		const std::string&		getIndex() const { return (_index); }
		const std::string&		getCgiPass() const { return (_cgiPass); }
		const config_map&		getConfigs() const { return (_configs); }

		friend std::ostream&	operator<<(std::ostream&, const Config&);
	private:
		LocationType				_lType;

		// 1) We must resolve the eventual localhost address
		// (case-insensitively: it must work with "LOCalhOST:80")
		// with /etc/host and "*" (or no address) with "0.0.0.0"
		// 2) We must add port 80 if no port is given
		// 3) After testing with NGINX, if no listen directive is
		// given, we must add "0.0.0.0:8000"
		listen_set					_listens;

		// 1) The names are case-insensitive, so it would be better
		// to record them in lowercase with str_tolower
		// 2) An explicit "" name is a valid empty name that must be added
		// to the vector as a empty string
		// 3) If no server_name is given, we must add an empty string name
		// to the vector, so the vector.size() cannot be 0
		// 4) An IP address is a valid name
		hostname_set				_serverNames;

		// TO DO: 1) Can only be defined once on a level, so the 2nd, 3rd...
		// definition lines will be ignored. For example these two lines ->
		// error_page   400 400 404 404  /50x.html;
		// error_page   400 402 403 404  /40x.html;
		// -> they result in a map with only two entries:
		// [400]="/50x.html", and [404]="/50x.html"
		// 2) The error_page must have (at least) two parameters (the last one
		// being the redirection), otherwise it must throw an exception (like
		// 'invalid number of arguments in "error_page" directive in
		// /usr/local/etc/nginx/nginx.conf:39')
		// 3) If the definition was inherited from the ServerConfig, the first
		// definition line replaces it (clear() the map, then record line)
		// 4) It must be a valid code as: 300 <= 'code' <= 599, otherwise exception
		// (like 'value "200" must be between 300 and 599 in /usr/.../nginx.conf:134')
		// 5) Internal redirection must not check the _errorPages to avoid an
		// infinite loop -> add variable indicating redirection? Or copy the location
		// and delete the _errorPages there?
		error_page_map				_errorPages;

		// 1) Can only be defined once on a level, and if another
		// definition line appears, it must throw an exception
		// (like '"client_max_body_size" directive is duplicate
		// in /usr/local/etc/nginx/nginx.conf:45')
		// 2) The max_body_size must have one parameter (the value),
		// otherwise it must throw an exception (like 'invalid number of
		// arguments in "client_max_body_size" directive in
		// /usr/local/etc/nginx/nginx.conf:41')
		// 3) The max_body_size parameter must be a valid unit ('' for bytes,
		// 'k' or 'K' for KB, 'm' or 'M' for MB) otherwise it must throw an
		// exception (like '"client_max_body_size" directive invalid value
		// in /usr/local/etc/nginx/nginx.conf:41')
		// 4) If the calculated size is more than LLONG_MAX, it must throw an
		// exception (like '"client_max_body_size" directive invalid value
		// in /usr/local/etc/nginx/nginx.conf:114')
		// 5) If the definition was inherited from the Config, the first
		// definition line replaces it
		int64_t						_maxBodySize;
		limit_except_set			_limitExcept;

		// 1) Can only be defined once on a level, so the 2nd, 3rd...
		// definition lines will be ignored, but this doesn't prevent the rest
		// of the location configuration to be parsed and checked for errors
		// 2) If no code is given, but only a full URL, the default code is 302
		// In this case (URL only), it must be a valid one (at least "http://"
		// or "https://", but test other edge cases) and it cannot be a URI local
		// to this server, otherwise, in either case, it must throw an exception
		// (like 'invalid return code "http:/" in /usr/etc/nginx/nginx.conf:128'),
		// 3) If giving a code, it must be: 0 <= 'code' <= 999, otherwise it
		// must throw an exception (like 'invalid return code "-1"/"1000"
		// in /usr/local/etc/nginx/nginx.conf:50')
		// 4) When giving a correct redirection code (301, 302, 303, 307, and 308),
		// if something else than the code is given, it is put in the "Location"
		// response header, and if it starts with a '/', it is taken as a local
		// URI relative to this server, which must be prefixed with:
		// "http://'_host'(host header in lowercase)[':_serverPort'](if not 80)"
		// (for example "http://localhost/abc", or "http://localhost:8080/abc")
		// 5) For other explicit codes, if something else than the code is given,
		// it becomes the response body text
		// 6) For the 444 code, if nothing else is given, it closes the connection
		// without sending a request, otherwise if some text is given, it is
		// treated as other codes (the text goes in the response body)
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
