#ifndef PARSECONFIG_HPP
#define PARSECONFIG_HPP

#include <fstream>
#include <stack>
#include <string>
#include <vector>

#include "config/ServerConfig.hpp"
#include "utils/ansi_colors.hpp"

namespace webserv {

class ServerConfig;

enum TokenType {
	kEOF = 0,
	kWord = 1,
	kBlockStart = 2,
	kBlockEnd = 3,
	kDirectiveEnd = 4,
	kComment = 5
};

class ParseConfig
{
    public:
		struct Token {
			Token(enum TokenType t, std::string v) : type(t), value(v) {}
			enum TokenType	type;
			std::string		value;
		};

		// struct Block {
		// 	std::string					path;
		// 	std::vector<struct Block>	locations;
		// 	std::vector<std::string>	directives;
		// };

		typedef std::map<int, std::string>					error_pages_map;
		typedef std::vector<struct Token>					token_list_t;
		typedef std::map<char, enum TokenType>				token_map_t;

        ParseConfig();
        ParseConfig(const std::string& configFilePath);
        ~ParseConfig();

		void    operator()();

		const token_list_t&					getTokens() const
												{ return (_tokens); }
		const std::string&					getConfigFilePath() const
												{ return (_configFilePath); }
		const std::ifstream&				getConfigFile() const
												{ return (_configFile); }
        const error_pages_map&				getErrorPages() const
                                            	{ return (_errorPages); }
		const long long&					getMaxBodySize() const
												{ return (_maxBodySize); }
		const std::vector<ServerConfig>&	getServerConfigs() const
												{ return (_serverConfigs); }

		friend std::ostream&	operator<<(std::ostream& os,
											const token_list_t& rhs);

    private:
        ParseConfig(const ParseConfig& src);
        ParseConfig&	operator=(const ParseConfig& rhs);

        std::string						_configFilePath;
        std::ifstream					_configFile;
	
		token_map_t						_tokenTypes;
		std::string						_delimiters;

		uint32_t						_nestedBlocks;
		uint32_t						_currentLine;
		std::string						_lineBuffer;
		token_list_t					_tokens;

		// std::stack<struct Block&>	_currentBlock;
		// std::vector<struct Block>	_virtualServers;

		error_pages_map					_errorPages;
		long long						_maxBodySize;
		std::vector<ServerConfig>		_serverConfigs;

		void	_initTokenMap();
		void	_addToken(const Token& token);
		void	_syntaxError(const Token& token, const char* expected);
		void	_extractWords(const std::string& buffer);
		void	_readLine();
		void	_lexer();
		void	_parser();

};

}	// namespace webserv

#endif	// PARSECONFIG_HPP