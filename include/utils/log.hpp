#ifndef LOGGER_HPP
# define LOGGER_HPP

# include <sys/stat.h>

# include <algorithm>
# include <exception>
# include <fstream>
# include <iostream>
# include <string>
# include <sstream>
# include <vector>

# include "webserv_config.hpp"

# define LOG(level, msg)	{												\
	try {																	\
		std::ostringstream	stream;											\
		std::string			str;											\
		stream << msg;														\
		str = stream.str();													\
		if (str.size() && *(str.rbegin()) == '\n')							\
			str.erase(str.end() - 1);										\
		Log::ContextInfo data(__FILE__, __LINE__, level);					\
		Log::Core::get().filter(data, str);									\
	} catch (const std::exception& exception) {								\
		std::cerr << "Logging error: " << exception.what() << std::endl;	\
	}																		\
}

# define LOG_EMERG(msg)		LOG(Log::Level::kEmerg, msg)
# define LOG_ERROR(msg)		LOG(Log::Level::kError, msg)
# define LOG_WARN(msg)		LOG(Log::Level::kWarn, msg)
# define LOG_INFO(msg)		LOG(Log::Level::kInfo, msg)
# define LOG_DEBUG(msg)		LOG(Log::Level::kDebug, msg)

namespace Log
{
	struct Level {
		enum l {
			kEmerg,		// System in an unusable state, need immediate attention
			kError, 	// Something was unsuccessful
			kWarn,		// Something unexpected but not concerning happened
			kInfo,		// Info messages, not necessary but good to know
			kDebug		// Useful debugging info to help locate the problem
		};
	};
	
	struct OutputStream {
		enum os {
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

	struct ContextInfo
	{
		ContextInfo(const std::string& f, const unsigned l, const unsigned lv)
				: file(f), line(l), level(lv)
		{ }

		const std::string	file;
		const unsigned int	line;
		const unsigned int	level;
	};

	class Sink {
	public:
		Sink(Level::l l) : _threshold(l) { }
		virtual ~Sink() { }

		virtual const std::string format(const ContextInfo& data,
											const std::string& msg) const = 0;
		virtual void write(const std::string& formattedOutput) = 0;

		Level::l	getThreshold() const { return (_threshold); }

	private:
		Level::l			_threshold;
	};

	class FileSink : public Sink {
	public:
		FileSink(Level::l level, const char* filePath);
		~FileSink();
		FileSink(const FileSink& src);

		const std::string	format(const ContextInfo& data,
									const std::string& msg) const;
		void 	write(const std::string& formattedOutput);

		const std::ofstream&	getLogfile() const { return (_logfile); }
	
	private:
		FileSink&	operator=(const FileSink& rhs);

		std::ofstream	_logfile;
		std::string		_filePath;
		long			_fileSize;
	};

	class ConsoleSink : public Sink {
	public:
		ConsoleSink(Level::l level);
		~ConsoleSink() { }
		ConsoleSink(const ConsoleSink& src);

		const std::string	format(const ContextInfo& data,
									const std::string& msg) const;
		void 	write(const std::string& formattedOutput);
	
	private:
		ConsoleSink&	operator=(const ConsoleSink& rhs);
	};

	class Core {
	public:
		static Core&	get()
		{
			static Core	instance;
			return (instance);
		}
		void	filter(const ContextInfo& data, const std::string& msg);

	private:
		Core();
		Core(const Core& src);
		virtual ~Core();

		Core&	operator=(const Core& rhs);

		Level::l			_threshold;
		OutputStream::os	_ostream;
		std::vector<Sink*>	_sinks;
	};

}	// namespace Log

#endif	// LOGGER_HPP
