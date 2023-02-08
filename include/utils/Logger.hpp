#ifndef LOGGER_HPP
# define LOGGER_HPP

# include <sstream>
# include <fstream>
# include <string>

# include "webserv_config.hpp"

# include <exception>
# include <iostream>

# define LOG(level, msg)	{												\
	try {																	\
		std::ostringstream	stream;											\
		stream << msg;														\
		webserv::Logger::getInstance().log(__FILE__, __LINE__,				\
											level, stream.str());			\
	} catch (const std::exception& e) {										\
		if (webserv::Logger::getInstance().getOutputStream()				\
				& webserv::Logger::kConsole)								\
			std::cerr << "Logging error: " << e.what() << std::endl;		\
		if (webserv::Logger::getInstance().getOutputStream()				\
				& webserv::Logger::kFile)									\
			webserv::Logger::getInstance().getLogfile()						\
				<< "Logging error: " << e.what() << std::endl;				\
	}																		\
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
			kEmerg,		// System in an unusable state, need immediate attention
			kError,		// Something was unsuccessful
			kWarn,		// Something unexpected but not concerning happened
			kInfo,		// Info messages, not necessary but good to know
			kDebug		// Useful debugging info to help locate the problem
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

		void	log(const std::string& file, unsigned int line,
						unsigned int level, const std::string& msg);

		std::ofstream&			getLogfile() { return (_logfile); }
		const OutputStream&		getOutputStream() const { return (_ostream); }

	private:
		Logger();
		Logger(const Logger& src);
		~Logger();

		Logger&	operator=(const Logger& rhs);

		struct ColorCode	_cc[5];
		LogLevel			_threshold;
		OutputStream		_ostream;
		std::ofstream		_logfile;
	};

}	// namespace webserv

#endif	// LOGGER_HPP
