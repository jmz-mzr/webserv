#ifndef CONFIGPARSER_HPP
#define CONFIGPARSER_HPP

#include <fstream>
#include <stack>
#include <string>
#include <deque>

#include "config/Lexer.hpp"
#include "config/ServerConfig.hpp"
#include "utils/ansi_colors.hpp"

namespace	webserv {

class	ServerConfig;

namespace	config {

	class	ConfigParser {
	public:
		// struct Block {
		// 	std::string					path;
		// 	std::vector<struct Block>	locations;
		// 	std::vector<std::string>	directives;
		// };

		typedef std::map<int, std::string>				error_pages_map;

		friend class Lexer;

		ConfigParser();
		ConfigParser(const std::string& filePath);
		~ConfigParser();

		void	operator()();

		const Lexer&						getLexer() const
												{ return (_lexer); }
		const error_pages_map&				getErrorPages() const
												{ return (_errorPages); }
		const long long&					getMaxBodySize() const
												{ return (_maxBodySize); }
		const std::vector<ServerConfig>&	getServerConfigs() const
												{ return (_serverConfigs); }

	private:
		ConfigParser(const ConfigParser& src);
		ConfigParser&	operator=(const ConfigParser& rhs);

		std::ifstream		_file;
		std::string			_filePath;
		std::string			_lineBuffer;
		uint32_t			_currentLineNb;

		Lexer				_lexer;

		// Token*						_currToken;
		// std::stack<struct Block&>	_currentBlock;
		// std::vector<struct Block>	_virtualServers;

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

}	// namespace config

}	// namespace webserv

#endif	// CONFIGPARSER_HPP
