#include <iostream>
#include "../include/http/HttpRequestHeader.hpp"

HttpRequestHeader::HttpRequestHeader() : buffer(), parsedRequest()
{
    index = 0;
}

HttpRequestHeader::HttpRequestHeader(const std::string &str) : parsedRequest()
{
    index = 0;
    buffer = str;
}

HttpRequestHeader::~HttpRequestHeader() { }

std::string HttpRequestHeader::readLine()
{
    std::string line;
    size_t      i;

    if (index == std::string::npos)
        return "";
    i = buffer.find_first_of('\n', index);
    if (i == std::string::npos)
        return "";
    line = buffer.substr(index, i - index);
    if (i == std::string::npos)
        index = i;
    else
        index = i + 1;
    return line;
}

void        HttpRequestHeader::parsePath(std::string line)
{
    size_t j ;
    std::string str;

    j = line.find_first_of(' ');
    str = line.substr(j + 1, line.size() - j + 1);
    j = str.find_first_of(' ');
    str = str.substr(0, j);
    path = str;
    parsedRequest.setPath(path);
}

void        HttpRequestHeader::parseMethod(std::string line)
{
    size_t  i;
    std::string str;

    i = line.find_first_of(' ');
    method.assign(line, 0, i);
    parsePath(line);
}

std::string        HttpRequestHeader::getKey(std::string line)
{
    size_t i;
    std::string ret;

    i = line.find_first_of(':');
    if (i == std::string::npos)
        return "";
    ret = line.substr(0, i);
    return ret;
}

std::string        HttpRequestHeader::getValue(std::string line)
{
    size_t i;
    std::string ret;

    i = line.find_first_of(':');
    if (i == std::string::npos)
        return "";
    ret = line.substr(i + 1, line.size() - i + 1);
    i = ret.find_first_of(' ');
    if (i == std::string::npos)
        return "";
    ret = ret.substr(i + 1, ret.size() - i + 1);
    return ret;
}

void        HttpRequestHeader::setBuffer(std::string buf)
{
    buffer = buf;
}

void        HttpRequestHeader::parse(std::string str)
{
    std::string line;
    std::string key;
    std::string value;

    buffer = str;
    parseMethod(readLine());
    while((line = readLine()) != "" && index != std::string::npos)
    {
        key = getKey(line);
        if (key == "")
            break ;
        value = getValue(line);
        if (value == "")
            break ;

        if (parsedRequest.findHeader(key))
            parsedRequest.setHeader(key, value);
    }
    std::cout << "path : " << path << std::endl;
    std::cout << "method : " << method << std::endl;
    parsedRequest.printHeader();
}