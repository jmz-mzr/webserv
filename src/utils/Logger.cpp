#include "utils/Logger.hpp"

#include <iostream>

#include "webserv_config.hpp"
#include "utils/ansi_colors.hpp"
#include "utils/bitwise_op.hpp"
#include "utils/global_defs.hpp"

namespace	webserv
{

	webserv::Logger& logger = webserv::Logger::getInstance();

	/**************************************************************************/
	/*                       CONSTRUCTORS / DESTRUCTORS                       */
	/**************************************************************************/

	Logger::Logger(): _threshold(LOG_LEVEL), _ostream(LOG_OSTREAM)
	{
		_cc[0].str = "EMERG", _cc[0].color = HMAG;
		_cc[1].str = "ERROR", _cc[1].color = HRED;
		_cc[2].str = "WARN", _cc[2].color = HYEL;
		_cc[3].str = "INFO", _cc[3].color = HWHT;
		_cc[4].str = "DEBUG", _cc[4].color = HCYN;
		if (_ostream & kFile)
		{
			_logfile.open(XSTR(LOG_FILE));
			if (!(_logfile.good())) {
				LOG_WARN("Cannot open \"" << XSTR(LOG_FILE) << "\", ignored");
				_ostream &= ~kFile;
			}
		}
	}

	Logger::~Logger()
	{
		if (_logfile.is_open()) {
			_logfile.close();
			if (!(_logfile.good())) {
				LOG_WARN("close(" << XSTR(LOG_FILE) << ") failed");
			} else {
				LOG_INFO("close(" << XSTR(LOG_FILE) << ")");
			}
		}
	}

	/**************************************************************************/
	/*                            MEMBER FUNCTIONS                            */
	/**************************************************************************/

	void	Logger::log(const std::string& file, unsigned int line,
						unsigned int level, const std::string& msg)
	{
		std::ostringstream	stream;
		std::string			output;

		if (_ostream != kNone && level <= _threshold) {
			stream << "webserv: [" << _cc[level].color << _cc[level].str << RESET
				<< "] " << BWHT << file << ":" << line << RESET
				<< ": " << msg;
			output = stream.str();
			if (_ostream & kConsole)
				std::cerr << output << std::endl;
			if (_ostream & kFile)
				_logfile << output << std::endl;
		}
	}

}	// namespace webserv
