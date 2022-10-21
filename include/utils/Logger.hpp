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

////////////////////////////////////////////////////////////////////////////////
//
//		Manage different level of logging information
//		Implemented as a Singleton
//		TODO: unit tests
//

public:
	static Logger& get_instance()
	{
		static Logger	instance;
		return (instance);
	}

	void
	log(std::string file, int line, int level, std::string msg);

private:
	Logger();
	Logger(const Logger& src);
	~Logger();

	Logger&
	operator=(const Logger& rhs);

	std::string
	format(std::string file, int line, int level, std::string msg);

	struct color_code {
		std::string	str;
		std::string	color;
	};

	struct color_code	cc[4];
	enum LogLevel		threshold;
	enum LogOutput		channel;
	std::ofstream		logfile;

};	/* class Logger */

}	/* namespace wsrv */

#endif /* LOGGER_HPP */
