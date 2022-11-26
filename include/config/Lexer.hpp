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

		struct	Token {
			enum	Type {
				kEOF = 0,
				kWord = 1,
				kBlockStart = 2,
				kBlockEnd = 3,
				kDirectiveEnd = 4,
				kComment = 5
			};

			Token(enum Type t, std::string v) : type(t), value(v) {}

			enum Type		type;
			std::string		value;
		};

		typedef std::deque<struct Token>			token_queue;
		typedef std::map<char, enum Token::Type>	token_dict;

		Lexer();
		~Lexer();

		void					operator()(const std::string& lineBuffer);
		friend std::ostream&	operator<<(std::ostream&, const token_queue&);

		const token_queue&		getTokens() const { return (_tokens); }

		bool	isEof;

	private:
		Lexer(const Lexer& src);

		Lexer&					operator=(const Lexer& rhs);

		void	_addToken(const Token& token);
		void	_extractWords(const std::string& buffer);
		void	_syntaxError(const Token& token, const char* expected);

		token_dict		_tokenTypes;
		std::string		_delimiters;
		token_queue		_tokens;
		unsigned		_nestedBlockCount;

	};

}	// namespace config

}	// namespace webserv

#endif /* LEXER_HPP */
