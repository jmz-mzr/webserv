# include <stdexcept>

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

	class	SyntaxErrorException: public LogicErrorException {
	public:
		SyntaxErrorException(const std::string& msg = "A syntax error occured")
				: LogicErrorException(msg) { }
	};

}	// namespace webserv