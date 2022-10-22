#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <fstream>
#include <string>

#define	LOG(level, msg)	\
webserv::Logger::getInstance().log(__FILE__, __LINE__, level, msg)

namespace webserv {

enum LogLevel {
	kNone = -1,
	kError = 0,
	kWarn = 1,
	kInfo = 2,
	kDebug = 3
};

enum LogOutput {
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
	Logger(const Logger& src);
	~Logger();

	Logger&	operator=(const Logger& rhs);

	std::string	format(std::string file, int line, int level, std::string msg);

	struct ColorCode	cc[4];
	enum LogLevel		threshold;
	enum LogOutput		channel;
	std::ofstream		logfile;

};

}	// namespace webserv

#endif	// LOGGER_HPP
