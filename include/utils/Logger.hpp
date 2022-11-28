#ifndef LOGGER_HPP
# define LOGGER_HPP

# include <sstream>
# include <fstream>
# include <string>

# define LOG(level, msg)	{													 \
	std::ostringstream stream;													 \
	stream << msg;																 \
	webserv::Logger::getInstance().log(__FILE__, __LINE__, level, stream.str()); \
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

	class	Logger {
	public:
		static Logger&	getInstance()
		{
			static Logger	instance;
			return (instance);
		}

		void	log(std::string file, int line,
					int level, const std::string& msg);

		const std::ofstream&		getLogfile() const { return (_logfile); }
		const LogOutput::Type&		getChannel() const { return (_channel); }
	private:
		Logger();
		Logger(const Logger& src);
		~Logger();

		Logger&	operator=(const Logger& rhs);

//		std::stringstream	format(std::string file, int line, int level);

		struct ColorCode	_cc[5];
		LogLevel::Level		_threshold;
		LogOutput::Type		_channel;
		std::ofstream		_logfile;
	};

}	// namespace webserv

#endif	// LOGGER_HPP
