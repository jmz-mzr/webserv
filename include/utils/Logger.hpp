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

# define LOG_EMERG(msg)		LOG(webserv::Logger::kEmerg, msg)
# define LOG_ERROR(msg)		LOG(webserv::Logger::kError, msg)
# define LOG_WARN(msg)		LOG(webserv::Logger::kWarn, msg)
# define LOG_INFO(msg)		LOG(webserv::Logger::kInfo, msg)
# define LOG_DEBUG(msg)		LOG(webserv::Logger::kDebug, msg)

namespace	webserv
{

	struct	ColorCode {
		std::string	str;
		std::string	color;
	};

	class	Logger {
	public:
		enum	LogLevel {
			kEmerg,		// The system is in an unusable state and requires immediate attention
			kError, 	// Something was unsuccessful
			kWarn,		// Something unexpected happened, however is not a cause for concern
			kInfo,		// Informational messages that aren't necessary to read but may be good to know
			kDebug		// Useful debugging information to help determine where the problem lies
		};

		enum	OutputStream {
			kNone = 0x00,
			kConsole = 0x01,
			kFile = 0x10,
			kBoth = 0x11
		};

		static Logger&	getInstance()
		{
			static Logger	instance;
			return (instance);
		}

		void	log(std::string file, int line,
					int level, const std::string& msg);

		const std::ofstream&	getLogfile() const { return (_logfile); }
		const OutputStream&		getOutputStream() const { return (_ostream); }
	private:
		Logger();
		Logger(const Logger& src);
		~Logger();

		Logger&	operator=(const Logger& rhs);

//		std::stringstream	format(std::string file, int line, int level);

		struct ColorCode	_cc[5];
		LogLevel			_threshold;
		OutputStream		_ostream;
		std::ofstream		_logfile;
	};

}	// namespace webserv

#endif	// LOGGER_HPP
