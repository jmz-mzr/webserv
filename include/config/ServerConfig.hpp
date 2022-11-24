#ifndef SERVERCONFIG_HPP
# define SERVERCONFIG_HPP

# include <utility>
# include <string>
# include <map>
# include <vector>

# include <stdint.h>

# include "utils/utils.hpp"
# include "config/ParseConfig.hpp"
# include "config/Location.hpp"

namespace	webserv
{

	class	ParseConfig;
	class	Location;

	class	ServerConfig {
	public:
		typedef std::pair<const std::string, uint16_t>		listen_pair;
		typedef std::map<int, std::string>					error_pages_map;
		typedef std::map<std::string, Location,
							location_compare>				locations_map;

		ServerConfig(const ParseConfig& src);
		ServerConfig(const ServerConfig& src);
		~ServerConfig() { }

		const std::vector<listen_pair>&	getListenPairs() const
													{ return (_listenPairs); }
		const std::vector<std::string>&	getServerNames() const
													{ return (_serverNames); }
		const error_pages_map&			getErrorPages() const
													{ return (_errorPages); }
		const long long&				getMaxBodySize() const
													{ return (_maxBodySize); }
		const locations_map&			getLocations() const
													{ return (_locations); }

		bool	empty() { return (_listenPairs.empty()); }

		void	clearListenPairs() { _listenPairs.clear(); }
		bool	eraseName(const std::string& name);
		void	clearConfig();

		void	addListenPair(const listen_pair& listenPair);
		void	addName(const std::string& name);
	private:
		ServerConfig();

		ServerConfig&	operator=(const ServerConfig& rhs);

		// TO DO: 1) We must resolve the eventual localhost address
		// (case-insensitively: it must work with "LOCalhOST:80")
		// with /etc/host and "*" (or no address) with "0.0.0.0"
		// 2) We must add port 80 if no port is given
		// 3) After testing with NGINX, if no listen directive is
		// given, we must add "0.0.0.0:8000"
		std::vector<listen_pair>	_listenPairs;

		// TO DO: 1) The names are case-insensitive, so it would be better
		// to record them in lowercase with str_tolower
		// 2) An explicit "" name is a valid empty name that must be added
		// to the vector as a empty string
		// 3) If no server_name is given, we must add an empty string name
		// to the vector, so the vector.size() cannot be 0
		// 4) An IP address is a valid name
		std::vector<std::string>	_serverNames;

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
		// 3) If the definition was inherited from the Config, the first
		// definition line replaces it (clear() the map, then record line)
		error_pages_map				_errorPages;

		// TO DO: 1) Can only be defined once on a level, and if another
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
		long long					_maxBodySize;

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
		// 5) For the sake of simplicity, don't allow nested locations
		locations_map				_locations;
	};

}	// namespace webserv

#endif	// SERVERCONFIG_HPP
