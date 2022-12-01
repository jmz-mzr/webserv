#ifndef PARSER_HPP
#define PARSER_HPP

#include <list>
#include <stack>
#include <string>
#include <vector>

#include "config/Lexer.hpp"
#include "config/Location.hpp"
//#include "config/Config.hpp"

namespace	webserv {

	class	Location;
	class	ServerConfig;

namespace	config {

	class	ConfigParser;
	class	Lexer;

	class Parser {
		public:
			Parser();
			~Parser();

			struct Directive {
				enum Type {
					kServerBlock,
					kLocationBlock,
					kServerName,
					kListen,
					kErrorPage,
					kLimitExcept,
					kRoot,
					kIndex,
					kBodyBufSize,
					kReturn,
					kAutoindex,
					kFastcgi
				}				type;
				enum DupRule {
					kIgnore,
					kForbid,
					kAccept
				}				dupRule;
				enum Context {
					kServer,
					kLocation,
					kBoth
				}				context;
				size_t			argc;
			};

			void	operator()(Lexer::token_queue& tokens);

			void	createServer();
			void	createLocation();



		private:
			Lexer::Token*						_currToken;
			Location*							_currLocation;
			ServerConfig*						_currServer;
			std::map<std::string, Directive>	_syntaxRules;
	};

}	// namespace config

}	// namespace webserv

#endif	// PARSER_HPP
