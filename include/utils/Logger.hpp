#ifndef LOGGER_HPP
# define LOGGER_HPP

# include <algorithm>
# include <fstream>
# include <list>
# include <string>
# include <sstream>

# include "webserv_config.hpp"

# include <exception>
# include <iostream>

# define LOG(level, msg)	{												\
	try {																	\
		std::ostringstream	stream;											\
		stream << msg;														\
		webserv::Logger::getInstance().log(__FILE__, __LINE__,				\
											level, stream.str());			\
	} catch (const std::exception& exception) {								\
		std::cerr << "Logging error: " << exception.what() << std::endl;	\
	}																		\
}

# define LOG_EMERG(msg)		LOG(webserv::LogLevel::kEmerg, msg)
# define LOG_ERROR(msg)		LOG(webserv::LogLevel::kError, msg)
# define LOG_WARN(msg)		LOG(webserv::LogLevel::kWarn, msg)
# define LOG_INFO(msg)		LOG(webserv::LogLevel::kInfo, msg)
# define LOG_DEBUG(msg)		LOG(webserv::LogLevel::kDebug, msg)

namespace	webserv
{

	struct LogLevel {
		enum Level {
			kEmerg,		// The system is in an unusable state and requires immediate attention
			kError, 	// Something was unsuccessful
			kWarn,		// Something unexpected happened, however is not a cause for concern
			kInfo,		// Informational messages that aren't necessary to read but may be good to know
			kDebug		// Useful debugging information to help determine where the problem lies
		};
	};
	
	struct LogOutput {
		enum Type {
			kNone = 0x00,
			kConsole = 0x01,
			kFile = 0x10,
			kBoth = 0x11
		};
	};

	struct	ColorCode {
		std::string	str;
		std::string	color;
	};

	class LogHandler {
	protected:
		LogHandler(LogOutput::Type stream, LogLevel::Level level);
		LogHandler&	operator=(const LogHandler& rhs);

		LogOutput::Type		_ostream;
		LogLevel::Level		_threshold;

	public:
		LogHandler(const LogHandler& src);
		virtual ~LogHandler() { }
		virtual void	writeMessage(const std::string& file,
										unsigned int line,
										unsigned int level,
										const std::string& str) = 0;
		const LogLevel::Level& getThreshold() const { return (_threshold); }
	};

	class FileLogger : public LogHandler {
	public:
		FileLogger(LogLevel::Level level, const char* filePath);
		~FileLogger();
		FileLogger(const FileLogger& src);

		void	writeMessage(const std::string& file,
								unsigned int line,
								unsigned int level,
								const std::string& str);
		const std::ofstream&	getLogfile() const { return (_logfile); }
	
	private:
		FileLogger&	operator=(const FileLogger& rhs);

		std::ofstream	_logfile;
	};

	class ConsoleLogger : public LogHandler {
	public:
		ConsoleLogger(LogLevel::Level level);
		~ConsoleLogger() { };
		ConsoleLogger(const ConsoleLogger& src);

		void	writeMessage(const std::string& file,
								unsigned int line,
								unsigned int level,
								const std::string& str);
	
	private:
		ConsoleLogger&	operator=(const ConsoleLogger& rhs);
	};

	class	Logger {
	public:
		static Logger&	getInstance()
		{
			static Logger	instance;
			return (instance);
		}

		void	log(const std::string& file, unsigned int line,
						unsigned int level, const std::string& msg);

	private:
		Logger();
		Logger(const Logger& src);
		~Logger();

		Logger&	operator=(const Logger& rhs);

		LogLevel::Level			_threshold;
		LogOutput::Type			_ostream;
		std::list<LogHandler *>	_logHandlers;
	};

}	// namespace webserv

#endif	// LOGGER_HPP
