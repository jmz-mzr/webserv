#ifndef CONFIG_HPP
# define CONFIG_HPP

# include <map>
# include <string>
# include <fstream>
# include <vector>
# include <exception>

# include "config/ServerConfig.hpp"
# include "utils/utils.hpp"

namespace	webserv
{

	class	ServerConfig;

	class	Config {
	public:
		typedef std::map<const int, std::string>	error_pages_map;

		Config();
		~Config() { closeConfigFile(); }

		void		openConfigFile(const std::string& configFileName);
		void		parseConfig();
		void		closeConfigFile();

		const std::ifstream&				getConfigFile() const
													{ return (_configFile); }
		const error_pages_map&				getErrorPages() const
													{ return (_errorPages); }
		const long long&					getMaxBodySize() const
													{ return (_maxBodySize); }
		const std::vector<ServerConfig>&	getServerConfigs() const
													{ return (_serverConfigs); }
	private:
		Config(const Config& src);

		Config&	operator=(const Config& rhs);

		std::string						_configFileName;
		std::ifstream					_configFile;

		// TO DO: 1) Can only be defined once on a level, so the 2nd, 3rd...
		// definition lines will be ignored. For example these two lines ->
        // error_page   400 400 404 404  /50x.html;
        // error_page   400 402 403 404  /40x.html;
		// -> they result in a map with only two entries ->
		// [400]="/50x.html", and [404]="/50x.html"
		// 2) The error_page must have (at least) two parameters (the last one
		// being the redirection), otherwise it must throw an exception (like
		// 'invalid number of arguments in "error_page" directive in
		// /usr/local/etc/nginx/nginx.conf:39')
		error_pages_map					_errorPages;

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
		long long						_maxBodySize;

		std::vector<ServerConfig>		_serverConfigs;
	};

}	// namespace webserv

#endif	// CONFIG_HPP
