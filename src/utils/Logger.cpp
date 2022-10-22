#include "utils/Logger.hpp"

#include <iostream>
#include <sstream>

#include "doctest/doctest.h"

#include "config.hpp"
#include "utils/ansi_colors.hpp"
#include "utils/bitwise_op.hpp"

namespace webserv {

webserv::Logger& logger = webserv::Logger::getInstance();

Logger::Logger() : threshold(CONF_LOG_LVL), channel(CONF_LOG_OUT)
{
	cc[0].str = "ERROR", cc[0].color = RED;
	cc[1].str = "WARN", cc[1].color = YEL;
	cc[2].str = "INFO", cc[2].color = WHT;
	cc[3].str = "DEBUG", cc[3].color = WHT;

	if (channel & kFile)
	{
		logfile.exceptions(std::ofstream::failbit | std::ofstream::badbit);
		try {
			logfile.open(CONF_LOG_FILE);
		} catch (std::ofstream::failure &e) {
			log(__FILE__, __LINE__, kError, e.what());
			channel &= ~kFile;
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
	return *this;
}

std::string
Logger::format(std::string file, int line, int level, std::string msg)
{
	std::stringstream stream;

	stream << "[" << cc[level].color << cc[level].str << RESET
			<< "]\t" + file << " (line " << line << "):\t" << msg;
	return stream.str();
}

void	Logger::log(std::string file, int line, int level, std::string msg)
{
	std::string output;

	if ( (!((level < kInfo) || (level > kDebug)) ) && (level <= threshold))
	{
		output = format(file, line, level, msg);
		if (channel & kConsole)
			logfile << output << std::endl;
		if (channel & kFile)
			std::cerr << output << std::endl;
	}
}

const std::ofstream&	Logger::getLogfile() const { return logfile; }
const enum LogOutput&	Logger::getChannel() const { return channel; }


////////////////////////////////////////////////////////////////////////////////
//
//		UNIT TESTS
//

TEST_SUITE("Logger class") {

	Logger &L = Logger::getInstance();

	TEST_CASE("get_instance() function") {
		Logger	&L1 = Logger::getInstance();
		REQUIRE_EQ(&L1, &L);
	}

	TEST_CASE("attribute value") {
		if (CONF_LOG_OUT & kFile) {
			const std::ofstream& fstream = L.getLogfile();
			CHECK_EQ(fstream, CONF_LOG_FILE);
			CHECK(fstream.is_open());
		// } else {
		// 	enum const webserv::LogOuput& chan = L.get_channel();
		// 	CHECK(chan & kConsole);
		// }
		}
	}
}
}	// namespace webserv