#ifndef LOGGER_HPP
#define LOGGER_HPP

#include "config.hpp"
#include "ansi_colors.hpp"

#include <string>
#include <iostream>
#include <fstream>
#include <sstream>

namespace wsrv {

class Logger
{

public:
	static Logger& get_instance()
	{
		static Logger	instance;
		return (instance);
	}

	void
	log(std::string file, int line, std::string level, std::string msg);

private:
	Logger();
	~Logger();

	Logger(const Logger& src);

	Logger&
	operator=(const Logger& rhs);

	struct level {
		std::string	str;
		std::string	color;
	};

	enum LogLevel	level;
	enum LogOutput	output;
	std::ofstream	logfile;

};	/* class Logger */

}	/* namespace wsrv */

#endif /* LOGGER_HPP */
