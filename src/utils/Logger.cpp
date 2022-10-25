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
	cc[0].str = "ERROR", cc[0].color = BHRED;
	cc[1].str = "WARN", cc[1].color = BHYEL;
	cc[2].str = "INFO", cc[2].color = BHWHT;
	cc[3].str = "DEBUG", cc[3].color = BBLU;

	if (channel & kFile)
	{
		logfile.exceptions(std::ofstream::failbit | std::ofstream::badbit);
		try {
			logfile.open(CONF_LOG_FILE);
		} catch (std::ofstream::failure &e) {
			std::string	msg(e.what());
			log(__FILE__, __LINE__, kError, msg);
			channel &= ~kFile;
		}
	}
}

Logger::~Logger() { }

void	Logger::log(std::string file, int line, int level, std::string msg)
{
	std::stringstream	stream;
	std::string			output;

	if (!(channel == kNone) && (level <= threshold)) {
		stream << "[" << cc[level].color << cc[level].str << RESET
				<< "]\t" + file << ":" << line << ": " << BHWHT << msg << RESET;
		output = stream.str();
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

	TEST_CASE("getInstance() function") {
		Logger	&L1 = Logger::getInstance();
		REQUIRE_EQ(&L1, &L);
	}

	TEST_CASE("Output channel") {
		if (CONF_LOG_OUT & kFile) {
			SUBCASE("file") {
				const std::ofstream& fstream = L.getLogfile();
				CHECK(fstream.is_open());
			}
		}
	}
}

}	// namespace webserv