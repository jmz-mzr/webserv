#ifndef PARSER_HPP
# define PARSER_HPP

# include <netinet/in.h>	// sockaddr_in
# include <stddef.h>		// size_t

# include <list>
# include <map>
# include <set>
# include <stack>
# include <string>
# include <vector>

# include "config/Config.hpp"
# include "config/Lexer.hpp"
# include "utils/utils.hpp"

namespace	webserv {

namespace	config {

	class	Parser {
	public:
		typedef Lexer::token_queue::iterator	it_t;

		static const size_t		kDirectiveNb = 16;

		enum Type {
			kErrorPage,
			kMaxBodySize,
			kLimitExcept,
			kIgnoreExcept,
			kReturn,
			kRoot,
			kAlias,
			kHideDirectory,
			kHideLimitRule,
			kAutoindex,
			kIndex,
			kCgiPass,
			kServerName,
			kListen,
			kServer,
			kLocation
		};

		enum Rule {
			kBlock = 0x00000100,
			kDirective = 0x00000200,
			kType = 0x00000700,			// mask
			kIgnoreDup = 0x00001000,	// only needed by single definitions
			kForbiddenDup = 0x00002000,
			kAcceptDup = 0x00004000,
			kDuplicate = 0x00007000,	// mask
			kArgcStrict = 0x00008000,
			kServCtx = 0x00010000,
			kLocCtx = 0x00020000,
			kNoCtx = 0x00040000,
			kContext = 0x00070000		// mask
		};

		struct Directive;

		struct DirectiveSyntax {
			typedef void (Parser::*parserMemFn)(Directive&);

			enum Type			type;
			int					rules;
			size_t				argc;
			std::string			str;
			parserMemFn			parseFn;
		};

		struct Directive {
			std::string&				name;
			it_t&						ctrlToken;
			std::vector<std::string>	argv;
			DirectiveSyntax&			syntax;

			Directive(it_t& first, it_t& last, DirectiveSyntax& syntax);
		};

		struct ConfigData {
			enum Type		type;
			Config&			config;
			bool			isDefined[Parser::kDirectiveNb];

			ConfigData(Type t, Config& conf);
		};

		Parser();
		~Parser() { };

		void						operator()(Lexer::token_queue& tokens);

		const std::list<Config>&	getConfigs() const { return (_configs); }
	private:
		std::list<Config>						_configs;
		std::stack<ConfigData>					_configStack;
		Config*									_currConfig;
		Directive*								_currDirectivePtr;
		std::set<std::string>					_methods;
		std::map<std::string, DirectiveSyntax>	_grammar;

		void	_parseDirective(it_t nameToken, it_t ctrlToken);
		void	_parseType(const Directive& directive);
		void	_parseContext(const Directive& directive);
		void	_parseDup(const Directive& directive);
		void	_parseArgc(const Directive& directive);

		void	_parseHost(const std::string&, std::list<sockaddr_in>& );
		void	_parseAddress(const std::string&, std::list<sockaddr_in>& );
		int		_parsePort(const std::string&, std::list<sockaddr_in>& );
		void	_duplicateCheck(const std::string& currDirStr,
								const size_t otherDirType,
								const std::string& otherDirStr);

		void	_addErrorPage(Directive& currDirective);
		void	_setMaxBodySize(Directive& currDirective);
		void	_setLimitExcept(Directive& currDirective);
		void	_setIgnoreExcept(Directive& currDirective);
		void	_setReturnPair(Directive& currDirective);
		void	_setRoot(Directive& currDirective);
		void	_setAlias(Directive& currDirective);
		void	_setHideDirectory(Directive& currDirective);
		void	_setHideLimitRule(Directive& currDirective);
		void	_setAutoIndex(Directive& currDirective);
		void	_setIndex(Directive& currDirective);
		void	_setCgiPass(Directive& currDirective);
		void	_addListen(Directive& currDirective);
		void	_addServerName(Directive& currDirective);
		void	_addLocation(Directive& currDirective);
		void	_addServer(Directive& currDirective);
	};

}	// namespace config

}	// namespace webserv

#endif	// PARSER_HPP
