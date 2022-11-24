#ifndef WEBSERV_CONFIG_HPP
# define WEBSERV_CONFIG_HPP

# ifndef CONF_LOG_LVL
#  define CONF_LOG_LVL		kError
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

# define EMPTY	0
# define GET	1
# define POST	2
# define DELETE	3

# ifndef WEBSERV_ROOT
#  define WEBSERV_ROOT		"/usr/local/var/www"
# endif

# if (defined(__GLIBCXX__) || defined(__GLIBCPP__))
#  define MACOS
# else
#  define LINUX
# endif

#endif	// WEBSERV_CONFIG_HPP
