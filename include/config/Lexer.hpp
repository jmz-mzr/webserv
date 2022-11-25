#ifndef LEXER_HPP
#define LEXER_HPP

#include <deque>
#include <map>
#include <string>

# include <stdint.h>

namespace	webserv {

namespace	config {

class	ConfigParser;

	class	Lexer {
	public:
		enum	TokenType {
			kEOF = 0,
			kWord = 1,
			kBlockStart = 2,
			kBlockEnd = 3,
			kDirectiveEnd = 4,
			kComment = 5
		};

		struct	Token {
			Token(enum TokenType t, std::string v) : type(t), value(v) {}
			enum TokenType	type;
			std::string		value;
		};

		typedef std::deque<struct Token>			token_queue;
		typedef std::map<char, enum TokenType>		token_dict;

		Lexer(const ConfigParser& src);
		~Lexer();

		const token_queue&		getTokens() const { return (_tokens); }

		void					operator()();
		friend std::ostream&	operator<<(std::ostream& os,
											const token_queue& rhs);
	private:
		Lexer();

		void	_addToken(const Token& token);
		void	_extractWords(const std::string& buffer);
		void	_syntaxError(const Token& token, const char* expected);

		token_dict		_tokenTypes;
		std::string		_delimiters;
		token_queue		_tokens;
		unsigned		_nestedBlockCount;

		const ConfigParser&	_configParser;


	};

}	// namespace config

}	// namespace webserv

#endif	// LEXER_HPP