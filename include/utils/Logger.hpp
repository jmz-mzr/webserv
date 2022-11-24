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

# define LOG_EMERG(msg)		LOG(webserv::kEmerg, msg)
# define LOG_ERROR(msg)		LOG(webserv::kError, msg)
# define LOG_WARN(msg)		LOG(webserv::kWarn, msg)
# define LOG_INFO(msg)		LOG(webserv::kInfo, msg)
# define LOG_DEBUG(msg)		LOG(webserv::kDebug, msg)

namespace	webserv
{

	enum	LogLevel {
		kEmerg,
		kError,
		kWarn,
		kInfo,
		kDebug
	};

	enum	LogOutput {
		kNone = 0x00,
		kConsole = 0x01,
		kFile = 0x10,
		kBoth = 0x11
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

		const std::ofstream&	getLogfile() const { return (_logfile); }
		const enum LogOutput&	getChannel() const { return (_channel); }
	private:
		Logger();
		Logger(const Logger& src);
		~Logger();

		Logger&	operator=(const Logger& rhs);

//		std::stringstream	format(std::string file, int line, int level);

		struct ColorCode	_cc[5];
		enum LogLevel		_threshold;
		enum LogOutput		_channel;
		std::ofstream		_logfile;
	};

}	// namespace webserv

#endif	// LOGGER_HPP
