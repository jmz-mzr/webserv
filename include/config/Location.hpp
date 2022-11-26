#ifndef LOCATION_HPP
# define LOCATION_HPP

# include <map>
# include <string>
# include <set>
# include <utility>

# include "config/ServerConfig.hpp"
# include "utils/utils.hpp"

namespace	webserv
{

	class	ServerConfig;

	struct	Method {
		enum	Type {
			kEmpty,
			kGet,
			kPost,
			kDelete
		};
	};

	class	Location {
	public:
		typedef std::map<int, std::string>				error_pages_map;
		typedef std::set<Method::Type>					limit_except_set;
		typedef std::pair<const int, std::string>		return_pair;
		typedef std::map<std::string, Location,
							location_compare>			locations_map;

		Location(const ServerConfig& src);
		Location(const Location& src);
		~Location() { }

		const error_pages_map&		getErrorPages() const
												{ return (_errorPages); }
		const long long&			getMaxBodySize() const
												{ return (_maxBodySize); }
		const limit_except_set&		getLimitExcept() const
												{ return (_limitExcept); }
		const return_pair&			getReturnPair() const
												{ return (_return); }
		const std::string&			getRoot() const
												{ return (_root); }
		bool						isAutoIndex() const
												{ return (_autoIndex); }
		const std::string&			getIndex() const
												{ return (_index); }
		const std::string&			getFastCgiPass() const
												{ return (_fastCgiPass); }
		const locations_map&		getLocations() const
												{ return (_locations); }
	private:
		Location();

		Location&	operator=(const Location& rhs);

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
		error_pages_map			_errorPages;

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
		// 5) If the definition was inherited from the ServerConfig, the first
		// definition line replaces it
		long long				_maxBodySize;

		// TO DO: Same as with NGINX -> if the set is not empty, and the
		// method is not in the set, we deny the access and return a 403
		// error, keeping the connection alive
		limit_except_set		_limitExcept;

		// TO DO: 1) Can only be defined once on a level, so the 2nd, 3rd...
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
		return_pair				_return;

		// TO DO: 1) Can only be defined once, and if another definition line
		// appears, it must throw an exception (like '"root" directive is
		// duplicate in /usr/local/etc/nginx/nginx.conf:117')
		// 2) If it was not defined it must not stay empty, and like NGINX
		// it must be set to the default _root: "html"
		std::string				_root;

		// TO DO: Can only be defined once, and if another
		// definition line appears, it must throw an exception
		// (like '"autoindex" directive is duplicate in
		// /usr/local/etc/nginx/nginx.conf:37')
		bool					_autoIndex;

		// TO DO: 1) For the sake of simplicity, accept only one default file
		// to answer if the request is a directory (unlike NGINX's index)
		// 2) Like NGINX's, it always has a default _index: "index.html",
		// already set when creating a Location class
		// 3) Can be defined multiple times, and for the sake of simplicity,
		// if another definition line appears, it replaces the previous one
		std::string				_index;

		// TO DO: Can only be defined once, and if another definition line
		// appears, it must throw an exception (like '"fastcgi_pass" directive
		// is duplicate in /usr/local/etc/nginx/nginx.conf:109')
		std::string				_fastCgiPass;

		// TO DO: 1) Extension locations follow the form "\*\.(alnum|$|.|_|-)+"
		// (star, dot, and then any NON-EMPTY (total size > 2) combination of
		// alphanum/dollar/dot/underscore/dash)
		// 2) For the sake of simplicity, only allow nested locations if they
		// are extension locations in normal locations
		locations_map			_locations;
	};

}	// namespace webserv

#endif	// LOCATION_HPP
