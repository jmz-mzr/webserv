#ifndef CONFIGPARSER_HPP
#define CONFIGPARSER_HPP

#include <fstream>
#include <stdexcept>
#include <string>

#include "config/Config.hpp"
#include "config/Lexer.hpp"
#include "config/Parser.hpp"
#include "config/Location.hpp"
#include "config/ServerConfig.hpp"
#include "utils/ansi_colors.hpp"

namespace	webserv {

	class	ServerConfig;

	class	ConfigParser {
	public:
		typedef std::map<int, std::string>				error_pages_map;

		ConfigParser(const std::string& path);
		~ConfigParser();

		const std::ifstream&		getFile() const;
		const std::string&			getFilePath() const;
		const uint32_t&				getCurrentLineNb() const;

		const std::list<Config>&	parseFile();

	private:
		ConfigParser();
		ConfigParser(const ConfigParser& src);
		ConfigParser&	operator=(const ConfigParser& rhs);

		bool	_readline();

		std::ifstream		_file;
		std::string			_filePath;
		std::string			_lineBuffer;
		uint32_t			_currentLineNb;
		config::Lexer		_lexer;
		config::Parser		_parser;

	};

}	// namespace webserv

#endif // CONFIGPARSER_HPP
