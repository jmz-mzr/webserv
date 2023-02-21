#ifndef WEBSERV_CONFIG_HPP
# define WEBSERV_CONFIG_HPP

//TODO:	use configure and autoheader to populate this file

# ifndef LOG_LEVEL
#  define LOG_LEVEL			webserv::Logger::kDebug
# endif

# ifndef LOG_OSTREAM
#  define LOG_OSTREAM		webserv::Logger::kBoth
# endif

# ifndef LOG_FILE
#  define LOG_FILE			/tmp/webserv.log
# endif

# ifndef CONF_FILE
#  define CONF_FILE			/usr/local/etc/webserv/conf/test.conf
# endif

# ifndef WEBSERV_ROOT
#  define WEBSERV_ROOT		/usr/local/var/www
# endif

# define RECV_BUFFER_SIZE	8192	// 8KB
//# define RECV_BUFFER_SIZE	16384	// 16KB
//# define RECV_BUFFER_SIZE	32768	// 32KB

# define SEND_BUFFER_SIZE	524288	// 500KB
//# define SEND_BUFFER_SIZE	1048576	// 1MB
//# define SEND_BUFFER_SIZE	2097152 // 2MB

# define READ_BUFFER_SIZE	65536	// 64KB
//# define READ_BUFFER_SIZE	131072	// 128KB

# define AUTOINDEX_NAME_LEN	50

# if (defined(__GLIBCXX__) || defined(__GLIBCPP__))
#  define MACOS
# else
#  define LINUX
# endif

#endif	// WEBSERV_CONFIG_HPP
