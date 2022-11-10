#ifndef WEBSERV_CONFIG_HPP
# define WEBSERV_CONFIG_HPP

# ifndef CONF_LOG_LVL
#  define CONF_LOG_LVL		kDebug
# endif

# ifndef CONF_LOG_OUT
#  define CONF_LOG_OUT		kBoth
# endif

# define CONF_LOG_FILE		"webserv.log"

# define DEFAULT_CONF_FILE	"config/default.conf"

# define RECV_BUFFER_SIZE	8192
//# define SEND_BUFFER_SIZE	81920
# define SEND_BUFFER_SIZE	524288	// 500KB
//# define SEND_BUFFER_SIZE	1048576	// 1MB

# if (defined(__GLIBCXX__) || defined(__GLIBCPP__))
#  define MACOS			0
#  define LINUX			1
# else
#  define MACOS			1
#  define LINUX			0
# endif

#endif	// WEBSERV_CONFIG_HPP
