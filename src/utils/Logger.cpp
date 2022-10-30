#include "utils/Logger.hpp"

#include <iostream>

#include "webserv_config.hpp"
#include "utils/ansi_colors.hpp"
#include "utils/bitwise_op.hpp"

#include "doctest/doctest.h"

namespace	webserv
{

	webserv::Logger& logger = webserv::Logger::getInstance();

	/**************************************************************************/
	/*                       CONSTRUCTORS / DESTRUCTORS                       */
	/**************************************************************************/

	Logger::Logger(): _threshold(CONF_LOG_LVL), _channel(CONF_LOG_OUT)
	{
		_cc[0].str = "ERROR", _cc[0].color = BHRED;
		_cc[1].str = "WARN", _cc[1].color = BHYEL;
		_cc[2].str = "INFO", _cc[2].color = BHWHT;
		_cc[3].str = "DEBUG", _cc[3].color = BBLU;
		if (_channel & kFile)
		{
			_logfile.open(CONF_LOG_FILE);
			if ((_logfile.rdstate() & std::ofstream::failbit) != 0) {
				LOG_WARN("Cannot open \"" << CONF_LOG_FILE << "\", ignored");
				_channel &= ~kFile;
			}
		}
	}

	Logger::~Logger()
	{
		if (_logfile.is_open()) {
			_logfile.close();
			if ((_logfile.rdstate() & std::ofstream::failbit) != 0) {
				LOG_WARN("Bad close() on \"" << _logfile << "\"");
				return ;
			}
			LOG_INFO("close(" << CONF_LOG_FILE << ")");
		}
	}

	/**************************************************************************/
	/*                            MEMBER FUNCTIONS                            */
	/**************************************************************************/

	void	Logger::log(std::string file, int line,
						int level, const std::string& msg)
	{
		std::ostringstream	stream;
		std::string			output;

		if (_channel != kNone && level <= _threshold) {
			stream << "[" << _cc[level].color << _cc[level].str << RESET
				<< "]\t" + file << ":" << line << ": " << BHWHT << msg << RESET;
			output = stream.str();
			if (_channel & kConsole)
				_logfile << output << std::endl;
			if (_channel & kFile)
				std::cerr << output << std::endl;
		}
	}

}	// namespace webserv
