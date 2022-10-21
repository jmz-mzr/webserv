#ifndef TYPEDEFS_HPP
#define TYPEDEFS_HPP

#include "bitwise_op.hpp"

namespace wsrv {

enum SocketType {
	ST_ANY = 0x01,
	ST_LIST = 0x10,
	ST_CONN = 0x11
};

enum LogLevel {
	LL_NONE = -1,
	LL_ERROR = 0,
	LL_WARN = 1,
	LL_INFO = 2,
	LL_DEBUG = 3
};

enum LogOutput {
	LO_CONSOLE = 0x01,
	LO_FILE = 0x10,
	LO_BOTH = 0x11
};

#define	LOG(level, msg)	wsrv::Logger::get_instance().log(__FILE__, __LINE__, level, msg)

}	/* namespace wsrv */

#endif /* TYPEDEFS_HPP */
