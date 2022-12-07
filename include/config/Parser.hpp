#ifndef PARSER_HPP
#define PARSER_HPP

#include <list>
#include <map>
#include <stack>
#include <string>

#include "config/Config.hpp"
#include "config/Lexer.hpp"
#include "config/Location.hpp"
#include "config/ServerConfig.hpp"
#include "core/Socket.hpp"
#include "utils/utils.hpp"

namespace	webserv {

namespace	config {


	class Parser {
	public:
		typedef Lexer::token_queue::iterator			it_t;

		static const size_t		kDirectiveNb = 12;

		enum Type {
			kErrorPage = 0,
			kMaxBodySize = 1,
			kLimitExcept = 2,
			kReturn = 3,
			kRoot = 4,
			kAutoindex = 5,
			kIndex = 6,
			kFastCgiPass = 7,
			kServerName = 8,
			kListen = 9,
			kServer = 10,
			kLocation = 11
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
			Type			type;	
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
		std::stack<ConfigData>					_currConfig;
		std::set<std::string>					_methods;
		std::map<std::string, DirectiveSyntax>	_grammar;

		void	_parseDirective(it_t nameToken, it_t ctrlToken);
		void	_parseType(const Directive& directive);
		void	_parseContext(const Directive& directive);
		void	_parseDup(const Directive& directive);
		void	_parseArgc(const Directive& directive);
		void	_dupError(const std::string& str);
		void	_contextError(const std::string& str);
		void	_argcError(const std::string& str);

		bool	_resolveHost(const std::string& str, uint16_t& port);
		void	_parseListenUnit(const std::vector<std::string>& parts,
									uint16_t& port, in_addr& ipAddr);

		void	_addErrorPage(Directive& currDirective);
		void	_setMaxBodySize(Directive& currDirective);
		void	_setLimitExcept(Directive& currDirective);
		void	_setReturnPair(Directive& currDirective);
		void	_setRoot(Directive& currDirective);
		void	_setAutoIndex(Directive& currDirective);
		void	_setIndex(Directive& currDirective);
		void	_setFastCgiPass(Directive& currDirective);
		void	_addListen(Directive& currDirective);
		void	_addServerName(Directive& currDirective);
		void	_addLocation(Directive& currDirective);
		void	_addServer(Directive& currDirective);

	};

}	// namespace config

}	// namespace webserv

#endif // PARSER_HPP
