#ifndef WEBSERV_CONFIG_HPP
# define WEBSERV_CONFIG_HPP

# ifndef LOG_LEVEL
#  define LOG_LEVEL			Log::Level::kDebug
# endif

# ifndef LOG_OSTREAM
#  define LOG_OSTREAM		Log::OutputStream::kBoth
# endif

# ifndef LOG_FILE
#  define LOG_FILE			/tmp/webserv.log
# endif

# ifndef LOGFILE_SIZE_MAX
#  define LOGFILE_SIZE_MAX	(1 << 20) * 20
# endif

# ifndef CONF_FILE
#  define CONF_FILE			/usr/local/etc/webserv/default.conf
# endif

# ifndef WEBSERV_ROOT
#  define WEBSERV_ROOT		/usr/local/var/www/webserv
# endif

# ifndef CGI_SESSION
#  define CGI_SESSION		/tmp
# endif

# ifndef LIB_PERL_CGI
#  define LIB_PERL_CGI		/usr/local/lib/perl5
# endif

//# define RECV_BUFFER_SIZE	8192	// 8KB
//# define RECV_BUFFER_SIZE	16384	// 16KB
# define RECV_BUFFER_SIZE	32768	// 32KB

# define SEND_BUFFER_SIZE	524288	// 500KB
//# define SEND_BUFFER_SIZE	1048576	// 1MB
//# define SEND_BUFFER_SIZE	2097152 // 2MB

# define READ_BUFFER_SIZE	65536	// 64KB
//# define READ_BUFFER_SIZE	131072	// 128KB

# define AUTOINDEX_NAME_LEN	50

# if (defined(__GLIBCXX__) || defined(__GLIBCPP__))
#  define LINUX
# else
#  define MACOS
# endif

#endif	// WEBSERV_CONFIG_HPP
