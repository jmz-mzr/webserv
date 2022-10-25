#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <sstream>
#include <fstream>
#include <string>

#define	LOG(level, msg)	{std::stringstream stream;							\
stream << msg;																\
webserv::Logger::getInstance().log(__FILE__, __LINE__, level, stream.str());}

#define LOG_ERROR(msg)	LOG(webserv::kError, msg)
#define	LOG_WARN(msg)	LOG(webserv::kWarn, msg)
#define LOG_INFO(msg)	LOG(webserv::kInfo, msg)
#define LOG_DEBUG(msg)	LOG(webserv::kDebug, msg)

namespace webserv {

enum LogLevel {
	kError = 0,
	kWarn = 1,
	kInfo = 2,
	kDebug = 3
};

enum LogOutput {
	kNone = 0x00,
	kConsole = 0x01,
	kFile = 0x10,
	kBoth = 0x11
};

struct ColorCode {
	std::string	str;
	std::string	color;
};

class Logger
{

public:
	static Logger& getInstance()
	{
		static Logger	instance;
		return instance;
	}

	void log(std::string file, int line, int level, std::string msg);

	inline const std::ofstream&		getLogfile() const;
	inline const enum LogOutput&	getChannel() const;


private:
	Logger();
	~Logger();

	// std::stringstream	format(std::string file, int line, int level);

	struct ColorCode	cc[4];
	enum LogLevel		threshold;
	enum LogOutput		channel;
	std::ofstream		logfile;

};

}	// namespace webserv

#endif /* LOGGER_HPP */
