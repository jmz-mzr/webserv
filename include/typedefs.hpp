#ifndef TYPEDEFS_HPP
#define TYPEDEFS_HPP

namespace wsrv {

enum SocketType {
	ST_ANY = 0x01,
	ST_LIST = 0x10,
	ST_CONN = 0x11
};

enum LogLevel {
	LL_NONE,
	LL_WARN,
	LL_ERROR,
	LL_INFO,
	LL_DEBUG
};

enum LogOutput {
	LO_CONSOLE,
	LO_FILE,
	LO_BOTH
};

#define	LOG(level, msg)	wsrv::Logger::get_instance().log(__FILE__, __LINE__, level, msg)

}	/* namespace wsrv */

#endif /* TYPEDEFS_HPP */
