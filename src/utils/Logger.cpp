#include "utils/Logger.hpp"

#include <iostream>
#include <list>

#include "webserv_config.hpp"
#include "utils/ansi_colors.hpp"
#include "utils/bitwise_op.hpp"
#include "utils/global_defs.hpp"

namespace	webserv
{

	namespace {
		const struct ColorCode	_cc[5] = {
			{ "EMERG", HMAG },
			{ "ERROR", HRED },
			{ "WARN", HYEL },
			{ "INFO", HWHT },
			{ "DEBUG", HCYN }
		};
	}	// namespace

	webserv::Logger& logger = webserv::Logger::getInstance();

	/**************************************************************************/
	/*                       CONSTRUCTORS / DESTRUCTORS                       */
	/**************************************************************************/

	Logger::Logger() : _threshold(LOG_LEVEL), _ostream(LOG_OSTREAM)
	{
		if (_ostream & LogOutput::kFile) {
			FileLogger* fileLogger = new FileLogger(_threshold, XSTR(LOG_FILE));
			if (fileLogger->getLogfile().good()) {
				_logHandlers.push_back(fileLogger);
			} else {
				delete fileLogger;
			}
		}
		if (_ostream & LogOutput::kConsole) {
			_logHandlers.push_back(new ConsoleLogger(_threshold));
		}
	}

	Logger::~Logger()
	{
		for (std::list<LogHandler *>::iterator it = _logHandlers.begin();
				it != _logHandlers.end();
				it++) {
			delete (*it);
		}
	}

	LogHandler::LogHandler(LogOutput::Type stream, LogLevel::Level level)
			: _ostream(stream), _threshold(level)
	{ }

	FileLogger::FileLogger(LogLevel::Level level, const char* filePath)
			: LogHandler(LogOutput::kFile, level)
	{
		_logfile.open(filePath, std::fstream::app);
		if ( !(_logfile.good()) ) {
			std::ostringstream	stream;
			stream << "webserv: ["
				<< _cc[LogLevel::kWarn].color << _cc[LogLevel::kWarn].str
				<< RESET << "] " << BWHT << __FILE__ << ":"
				<< __LINE__ << RESET << ": Cannot open \"" << filePath
				<< "\", ignored" << std::endl;
			std::cerr << stream.str();
		}
	}

	FileLogger::~FileLogger()
	{
		if (_logfile.is_open()) {
			_logfile.close();
			if ( !(_logfile.good()) ) {
				LOG_WARN("close(" << XSTR(LOG_FILE) << ") failed");
			} else {
				LOG_INFO("close(" << XSTR(LOG_FILE) << ")");
			}
		}
	}

	ConsoleLogger::ConsoleLogger(LogLevel::Level level)
			: LogHandler(LogOutput::kConsole, level)
	{ }

	/**************************************************************************/
	/*                            MEMBER FUNCTIONS                            */
	/**************************************************************************/

	void	FileLogger::writeMessage(const std::string& file,
										unsigned int line,
										unsigned int level,
										const std::string& str)
	{
		std::ostringstream	stream;

		stream << "webserv: [" << _cc[level].str << "] "
				<< file << ":" << line << ": " << str << std::endl;
		_logfile << stream.str();
		_logfile.flush();
	}

	void	ConsoleLogger::writeMessage(const std::string& file,
										unsigned int line,
										unsigned int level,
										const std::string& str)
	{
		std::ostringstream	stream;

		stream << "webserv: [" << _cc[level].color << _cc[level].str
				<< RESET << "] " << BWHT << file << ":" << line
				<< RESET << ": " << str << std::endl;
		std::cerr << stream.str();
	}

	void	Logger::log(const std::string& file, unsigned int line,
						unsigned int level, const std::string& msg)
	{
		if (_ostream != LogOutput::kNone) {
			for (std::list<LogHandler *>::iterator it = _logHandlers.begin();
					it != _logHandlers.end();
					it++) {
				if (level <= static_cast<unsigned int>((*it)->getThreshold())) {
					(*it)->writeMessage(file, line, level, msg);
				}
			}
		}
	}

}	// namespace webserv
