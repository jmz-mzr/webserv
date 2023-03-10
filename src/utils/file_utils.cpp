#include <sys/stat.h>
#include <string>

namespace webserv {

    long	getFileSize(const std::string& filename)
    {
        struct stat stat_buf;
        int rc = stat(filename.c_str(), &stat_buf);
        return ( (rc == 0) ? stat_buf.st_size : -1 );
    }

    long	fdGetFileSize(int fd)
    {
        struct stat stat_buf;
        int rc = fstat(fd, &stat_buf);
        return ( (rc == 0) ? stat_buf.st_size : -1 );
    }

}	// namespace webserv
