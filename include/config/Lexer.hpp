#ifndef LEXER_HPP
# define LEXER_HPP

# include <stdint.h>

# include <deque>
# include <iostream>
# include <map>
# include <string>

namespace	webserv {

namespace	config {

	class	Lexer {
	public:

		struct	Token {
			enum	Type {
				kEOF = 0,
				kWord = 1,
				kComment = 2,
				kCtrlToken = 4,		// 00000100
				kBlockStart = 5,	// 00000101
				kBlockEnd = 6,		// 00000110
				kDirectiveEnd = 7	// 00000111
			};

			Token(Type t, std::string v) : type(t), value(v) {}

			Type			type;
			std::string		value;
		};

		typedef std::deque<struct Token>			token_queue;
		typedef std::map<char, enum Token::Type>	token_dict;

		Lexer();
		~Lexer();

		void					operator()(const std::string& lineBuffer);
		friend std::ostream&	operator<<(std::ostream&, const token_queue&);

		token_queue&			getTokens() { return (_tokens); }

		static bool				isNotWord(Lexer::Token tk);

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

#endif	// LEXER_HPP
