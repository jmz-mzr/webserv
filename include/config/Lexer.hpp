#ifndef LEXER_HPP
#define LEXER_HPP

#include <deque>
#include <map>
#include <string>

# include <stdint.h>

namespace	webserv {

namespace	config {
	
	typedef std::deque<struct Token>	token_queue;

	struct	Token {
		enum	Type {
			kWord = 0x00,
			kComment = 0x01,
			kBlockStart = 0x10,
			kBlockEnd = 0x11,
			kDirectiveEnd = 0x12,
			kEOF = 0x13,
		};

		Token(enum Type t, std::string v) : type(t), value(v) {}

		enum Type		type;
		std::string		value;
	};

	class	Lexer {
	public:
		typedef std::map<char, enum Token::Type>	token_dict;

		Lexer();
		~Lexer();

		void					operator()(const std::string& lineBuffer);
		friend std::ostream&	operator<<(std::ostream&, const token_queue&);

		token_queue&			getTokens() { return (_tokens); }

		bool	isEof;
		bool	isParseReady;
		
		static const uint8_t	kctrlTokenMask = 16U;

	private:
		Lexer(const Lexer& src);

		Lexer&	operator=(const Lexer& rhs);

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
