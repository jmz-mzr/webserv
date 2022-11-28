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
		_cc[0].str = "EMERG", _cc[0].color = HMAG;
		_cc[1].str = "ERROR", _cc[1].color = HRED;
		_cc[2].str = "WARN", _cc[2].color = HYEL;
		_cc[3].str = "INFO", _cc[3].color = HWHT;
		_cc[4].str = "DEBUG", _cc[4].color = HCYN;
		if (_channel & LogOutput::kFile)
		{
			_logfile.open(CONF_LOG_FILE);
			if (!(_logfile.good())) {
				LOG_WARN("Cannot open \"" << CONF_LOG_FILE << "\", ignored");
				_channel &= ~(LogOutput::kFile);
			}
		}
	}

	Logger::~Logger()
	{
		if (_logfile.is_open()) {
			_logfile.close();
			if (!(_logfile.good())) {
				LOG_WARN("close(" << _logfile << ") failed");
			} else {
				LOG_INFO("close(" << CONF_LOG_FILE << ")");
			}
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

		if (_channel != LogOutput::kNone && level <= _threshold) {
			stream << "webserv: [" << _cc[level].color << _cc[level].str << RESET
				<< "] " << BWHT << file << ":" << line << RESET
				<< ": " << msg;
			output = stream.str();
			if (_channel & LogOutput::kConsole)
				_logfile << output << std::endl;
			if (_channel & LogOutput::kFile)
				std::cerr << output << std::endl;
		}
	}

}	// namespace webserv