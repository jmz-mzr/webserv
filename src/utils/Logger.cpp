#include "Logger.hpp"

namespace wsrv {

wsrv::Logger& logger = wsrv::Logger::get_instance();

Logger::Logger()
: threshold(CONF_LOG_LVL), channel(CONF_LOG_OUT)
{
	cc[0].str = "ERROR", cc[0].color = RED;
	cc[1].str = "WARN", cc[1].color = YEL;
	cc[2].str = "INFO", cc[2].color = WHT;
	cc[3].str = "DEBUG", cc[3].color = WHT;

	if (CONF_LOG_OUT)
	{
		logfile.exceptions(std::ofstream::failbit | std::ofstream::badbit);
		try {
			logfile.open(CONF_LOG_FILE);
		} catch (std::ofstream::failure &e) {
			log(__FILE__, __LINE__, LL_ERROR, e.what());
			channel &= ~LO_FILE;
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
Logger::format(std::string file, int line, int level, std::string msg)
{
	std::stringstream stream;

	stream << "[" << cc[level].color << cc[level].str << RESET << "]\t" + file << " (line " << line << "):\t" << msg;
	return (stream.str());
}

void
Logger::log(std::string file, int line, int level, std::string msg)
{
	std::string output;

	if ( (!((level < LL_INFO) || (level > LL_DEBUG)) ) && (level <= threshold))
	{
		output = format(file, line, level, msg);
		if (channel & LO_CONSOLE)
			logfile << output << std::endl;
		if (channel & LO_FILE)
			std::cerr << output << std::endl;
	}
}

}	/* namespace wsrv */