#include "Logger.hpp"

namespace wsrv {

wsrv::Logger& logger = wsrv::Logger::get_instance();

Logger::Logger()
: level(CONF_LOG_LVL), output(CONF_LOG_OUT)
{
	if (CONF_LOG_OUT)
	{
		logfile.exceptions(std::ofstream::failbit | std::ofstream::badbit);
		try {
			logfile.open(CONF_LOG_FILE);
		} catch (std::ofstream::failure &e) {
			std::cerr << "[WARN]\t" << __FILE__ << ": line " << __LINE__  << "\t" << e.what() << std::endl;
		}
	}
	return;
}

Logger::Logger(const Logger& src)
{
	(void)src;
	return;
}

Logger::~Logger()
{ return; }

Logger&
Logger::operator=(const Logger& rhs)
{
	(void)rhs;
	return (*this);
}

std::string
Logger::format(std::string file, int line, std::string level, std::string msg)
{
	std::stringstream stream;
	struct lvl	lvls[4] =
	{
		{ "ERROR", RED },
		{ "WARN", YEL },
		{ "INFO", YEL },
		{ "DEBUG", YEL },
	};
	int i;

	for (i = 0; i < 4; i++)
		if (level == lvls[i].str)
			break;
	if (i <= CONF_LOG_LVL)
		stream << "[" << lvls[i].color << lvls[i].str << RESET << "]\t" + file << " (line " << line << "):\t" << msg;
	return (stream.str());
}

void
Logger::log(std::string file, int line, std::string level, std::string msg)
{
	std::string output;

	output = format(file, line, level, msg);
	if (!output.empty())
	{
		if (CONF_LOG_OUT)
			logfile << output << std::endl;
		else
			std::cerr << output << std::endl;
	}
}

}	/* namespace wsrv */