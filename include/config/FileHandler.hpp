#ifndef FILEHANDLER_HPP
#define FILEHANDLER_HPP

#include <deque>
#include <fstream>
#include <stdexcept>
#include <string>

#include "config/Lexer.hpp"
#include "config/Parser.hpp"
#include "utils/ansi_colors.hpp"

namespace	webserv {

namespace	config {

	class	FileHandler {
	public:
		FileHandler();
		FileHandler(const std::string& filePath);
		~FileHandler();

		void	parse();

		const std::ifstream&	getFile() const
									{ return (_file); }
		const std::string&		getFilePath() const
									{ return (_filePath); }
		const uint32_t&			getCurrentLineNb() const
									{ return (_currentLineNb); }

	private:
		FileHandler(const FileHandler& src);
		FileHandler&	operator=(const FileHandler& rhs);

		bool	_readline();

		std::ifstream				_file;
		std::string					_filePath;
		std::string					_lineBuffer;
		uint32_t					_currentLineNb;

		config::Lexer				_lexer;
		config::Parser				_parser;

	};

	class	SyntaxErrorException: public std::logic_error {
	public:
		SyntaxErrorException(const std::string& msg = "A syntax error occured")
				: logic_error(msg) { }
	};

}	// namespace config

}	// namespace webserv

#endif // FILEHANDLER_HPP
