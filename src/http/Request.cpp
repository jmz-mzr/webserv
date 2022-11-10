#include "http/Request.hpp"
#include <iostream>

Request::Request(/* args */)
{
    initHeader();
}

void    Request::initHeader()
{
    this->header["Accept"] = "";
    this->header["Accept-Charset"] = "";
    this->header["Accept-Encoding"] = "";
    this->header["Accept-Language"] = "";
    this->header["Authorization"] = "";
    this->header["Cache-Control"] = "";
    this->header["Connection"] = "";
    this->header["Content-Encoding"] = "";
    this->header["Content-Length"] = "";
    this->header["Content-Type"] = "";
    this->header["Date"] = "";
    this->header["From"] = "";
    this->header["Host"] = "";
    this->header["Transfer-Encoding"] = "";
    this->header["User-Agent"] = "";
}

bool    Request::findHeader(std::string key)
{
    if (header.count(key))
        return true;
    return false;
}

void    Request::setHeader(std::string key, std::string value)
{
    header[key] = value;
}

void    Request::setPath(std::string str)
{
    path = str;
}

void    Request::setCode(int i)
{
    code = i;
}

int     Request::getCode()
{
    return code;
}

void    Request::printHeader()
{
    std::map<std::string, std::string>::iterator it = header.begin();
    for ( ; it != header.end() ; it++)
    {
        std::cout << "[" << it->first << "]: ";
        std::cout << it->second << std::endl;
    }
}

Request::~Request()
{
}