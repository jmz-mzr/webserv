#include <cstring>
#include <stdexcept>

namespace webserv {

	class	FatalErrorException: public std::runtime_error {
	public:
		FatalErrorException(const std::string& msg = "A fatal error occured")
				: std::runtime_error(msg) 
		{ }

		FatalErrorException(int err, std::string msg = "A fatal error occured")
				: std::runtime_error(msg + strerror(err))
		{ }
	};

	class	LogicErrorException: public std::logic_error {
	public:
		LogicErrorException(const std::string& msg = "A logic error occured")
				: std::logic_error(msg)
		{ }
		
		LogicErrorException(int err, std::string msg)
				: std::logic_error(msg + strerror(err))
		{ }
	};

	class	SyntaxErrorException: public std::logic_error {
	public:
		SyntaxErrorException(const std::string& msg = "A syntax error occured")
				: std::logic_error(msg) { }
	};

}	// namespace webserv