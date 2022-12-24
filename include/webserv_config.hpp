#ifndef WEBSERV_CONFIG_HPP
# define WEBSERV_CONFIG_HPP

//TODO:	use configure and autoheader to populate this file

# ifndef LOG_LEVEL
#  define LOG_LEVEL			webserv::Logger::kDebug
# endif

# ifndef LOG_OSTREAM
#  define LOG_OSTREAM		webserv::Logger::kBoth
# endif

# if defined(LOG_OSTREAM) && !defined(LOG_FILE)
#  define LOG_FILE			webserv.log
# endif

//! this define doesn't work if package not installed yet
//! I'm using a temporary value until I fix this
# ifndef DEFAULT_CONF_FILE
#  define DEFAULT_CONF_FILE	conf/test.conf
# endif

# define RECV_BUFFER_SIZE	8192
//# define SEND_BUFFER_SIZE	81920
# define SEND_BUFFER_SIZE	524288	// 500KB
//# define SEND_BUFFER_SIZE	1048576	// 1MB

# if (defined(__GLIBCXX__) || defined(__GLIBCPP__))
#  define MACOS
# else
#  define LINUX
# endif

#endif	// WEBSERV_CONFIG_HPP
