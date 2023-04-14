#include <stdio.h>		// rename
#include <sys/stat.h>

#include <cerrno>		// errno
#include <cstring>		// strerror

#include <ios>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "utils/log.hpp"
#include "webserv_config.hpp"
#include "utils/ansi_colors.hpp"
#include "utils/bitwise_op.hpp"
#include "utils/exceptions.hpp"
#include "utils/global_defs.hpp"
#include "utils/utils.hpp"
#include "webserv_config.hpp"

namespace Log
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

	Core& logger = Core::get();

	/**************************************************************************/
	/*                       CONSTRUCTORS / DESTRUCTORS                       */
	/**************************************************************************/

	Core::Core() : _threshold(LOG_LEVEL), _ostream(LOG_OSTREAM)
	{
		_sinks.reserve(2);
		if (_ostream & OutputStream::kFile) {
			FileSink* fileLogger = new FileSink(_threshold, XSTR(LOG_FILE));
			if (fileLogger->getLogfile().good()) {
				_sinks.push_back(fileLogger);
			} else {
				delete fileLogger;
			}
		}
		if (_ostream & OutputStream::kConsole) {
			_sinks.push_back(new ConsoleSink(_threshold));
		}
	}

	Core::~Core()
	{
		for (std::vector<Sink*>::iterator it = _sinks.begin();
				it != _sinks.end();
				it++) {
			delete (*it);
		}
	}

	ConsoleSink::ConsoleSink(Level::l level)
			: Sink(level)
	{ }

	FileSink::FileSink(Level::l level, const char* filePath)
			: Sink(level), _filePath(filePath)
	{
		_logfile.open(filePath, std::ios_base::app);
		if ( !(_logfile.good()) ) {
			std::ostringstream	stream;
			stream << "webserv: ["
				<< _cc[Level::kWarn].color << _cc[Level::kWarn].str
				<< RESET << "] " << BWHT << __FILE__ << ":"
				<< __LINE__ << RESET << ": Cannot open \"" << filePath
				<< "\" (" << std::strerror(errno) << "), ignored" << std::endl;
			std::cerr << stream.str();
		} else {
			_fileSize = webserv::getFileSize(filePath);
			if (_fileSize == -1) {
				std::ostringstream	stream;
				stream << "webserv: ["
					<< _cc[Level::kWarn].color << _cc[Level::kError].str
					<< RESET << "] " << BWHT << __FILE__ << ":" << __LINE__
					<< RESET << ": " << std::strerror(errno) << std::endl;
				std::cerr << stream.str();
				throw (webserv::FatalErrorException());
			}
		}
	}

	FileSink::~FileSink()
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

	/**************************************************************************/
	/*                            MEMBER FUNCTIONS                            */
	/**************************************************************************/

	const std::string	FileSink::format(const ContextInfo& data,
											const std::string& msg) const
	{
		std::ostringstream	stream;
		char 				timestamp[32];

		std::strftime(timestamp, 32, "%d/%m/%Y - %H:%M:%S",
												std::localtime(&(data.time)));
		stream << "webserv: ["
				<< _cc[data.level].str << "] [" << timestamp << "] "
				<< data.file << ":" << data.line << ": " << msg << std::endl;
		return (stream.str());
	}

	void	FileSink::write(const std::string& formattedOutput)
	{
		if (_logfile.tellp() >= LOGFILE_SIZE_MAX) {
			_logfile.close();
			std::string backupPath = (_filePath + ".old").c_str();
			if (rename(_filePath.c_str(), backupPath.c_str()) == -1) {
				std::ostringstream	stream;
				stream << "webserv: ["
					<< _cc[Level::kWarn].color << _cc[Level::kWarn].str
					<< RESET << "] " << BWHT << __FILE__ << ":" << __LINE__
					<< RESET << ": " << std::strerror(errno) << std::endl;
				std::cerr << stream.str();
			} else {
				_logfile.open(_filePath.c_str());
				_fileSize = webserv::getFileSize(_filePath);
				if ( !(_logfile.good()) ) {
					std::ostringstream	stream;
					stream << "webserv: ["
						<< _cc[Level::kError].color << _cc[Level::kError].str
						<< RESET << "] " << BWHT << __FILE__ << ":" << __LINE__
						<< RESET << ": " << std::strerror(errno) << std::endl;
					std::cerr << stream.str();
					throw (webserv::FatalErrorException());
				}
			}
		}
		_logfile << formattedOutput;
		_logfile.flush();
	}

	const std::string	ConsoleSink::format(const ContextInfo& data,
												const std::string& msg) const
	{
		std::ostringstream	stream;
		char 				timestamp[16];

		std::strftime(timestamp, 16, "%H:%M:%S", std::localtime(&(data.time)));
		stream << "webserv: [" << _cc[data.level].color << _cc[data.level].str
				<< RESET << "] [" << timestamp << "] "
				<< BWHT << data.file << ":" << data.line
				<< RESET << ": " << msg << std::endl;
		return (stream.str());
	}

	void	ConsoleSink::write(const std::string& formattedOutput)
	{
		std::cerr << formattedOutput;
	}

	void	Core::filter(const ContextInfo& data, const std::string& msg)
	{
		if (_ostream != OutputStream::kNone) {
			for (std::vector<Sink*>::iterator it = _sinks.begin();
					it != _sinks.end();
					it++) {
				if (data.level <= (*it)->getThreshold()) {
					(*it)->write((*it)->format(data, msg));
				}
			}
		}
	}

	void	Core::closeLogFile()
	{
		FileSink*	fileSink;

		for (std::vector<Sink*>::iterator it = _sinks.begin();
				it != _sinks.end();
				++it) {
			fileSink = dynamic_cast<FileSink*>(*it);
			if (fileSink) {
				const_cast<std::ofstream&>(fileSink->getLogfile()).close();
				break ;
			}
		}
	}

}	// namespace Log
