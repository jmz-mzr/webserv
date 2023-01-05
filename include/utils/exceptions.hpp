#include <cstring>
#include <cerrno>
#include <stdexcept>

#include "utils/Logger.hpp"

#define THROW_FATAL(msg)	{												\
	LOG_ERROR(msg);															\
	throw FatalErrorException();											\
}

#define THROW_LOGIC(msg)	{												\
	LOG_EMERG(msg);															\
	throw LogicErrorException();											\
}

#define THROW_SYNTAX(msg)	{												\
	LOG_EMERG(msg);															\
	throw SyntaxErrorException();											\
}


namespace webserv {

	class	FatalErrorException: public std::runtime_error {
	public:
		FatalErrorException(const std::string& msg = "A fatal error occured")
				: std::runtime_error(msg) 
		{ }

	};

	class	LogicErrorException: public std::logic_error {
	public:
		LogicErrorException(const std::string& msg = "A logic error occured")
				: std::logic_error(msg)
		{ }

	};

	class	SyntaxErrorException: public std::logic_error {
	public:
		SyntaxErrorException(const std::string& msg = "A syntax error occured")
				: std::logic_error(msg)
		{ }
	};

}	// namespace webserv

