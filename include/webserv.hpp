#ifndef WEBSERV_HPP
#define WEBSERV_HPP

#include "utils/Logger.hpp"

#define	LOG(level, msg)	\
webserv::Logger::getInstance().log(__FILE__, __LINE__, level, msg)

#endif /* WEBSERV_HPP */
