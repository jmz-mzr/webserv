#ifndef LOCATION_HPP
# define LOCATION_HPP

# include <map>
# include <string>
# include <set>
# include <utility>

# include "config/ServerConfig.hpp"

namespace	webserv
{

	class	ServerConfig;

	class	Location {
	public:
		typedef std::map<const int, std::string>		error_pages_map;
		typedef std::set<int>							limit_except_set;
		typedef std::pair<const int, std::string>		return_pair;

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
		const std::string&			getTryFile() const
												{ return (_tryFile); }
		const std::string&			getFastCgiPass() const
												{ return (_fastCgiPass); }
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
		// method is not in the set, we deny the access
		limit_except_set		_limitExcept;

		// TO DO: 1) If no return code is given, but only a full URL, the
		// default code is 302
		// 2) Can only be defined once on a level, so the 2nd, 3rd...
		// definition lines will be ignored, but this doesn't prevent the rest
		// of the location configuration to be parsed and checked for errors
		// 3) Test all edges cases in comparison with NGINX
		return_pair				_return;

		std::string				_root;

		// TO DO: Can only be defined once, and if another
		// definition line appears, it must throw an exception
		// (like '"autoindex" directive is duplicate in
		// /usr/local/etc/nginx/nginx.conf:37')
		bool					_autoIndex;

		// TO DO: 1) Only one default file to answer if the request is
		// a directory (so different from NGINX's try_files)
		// 2) Can only be defined once, and if another definition line appears,
		// it must throw an exception (like '"try_file" directive is duplicate
		// in /usr/local/etc/nginx/nginx.conf:116')
		std::string				_tryFile;

		// TO DO: Can only be defined once, and if another definition line
		// appears, it must throw an exception (like '"fastcgi_pass" directive
		// is duplicate in /usr/local/etc/nginx/nginx.conf:109')
		std::string				_fastCgiPass;
	};

}	// namespace webserv

#endif	// LOCATION_HPP
