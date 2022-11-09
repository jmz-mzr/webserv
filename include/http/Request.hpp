#ifndef REQUEST_HPP
#define REQUEST_HPP

#include <map>

class Request
{
private:
    void    initHeader();
    std::map<std::string, std::string> header;
public:
    bool    findHeader(std::string key);
    bool    setHeader(std::string key, std::string value);
    Request(/* args */);
    ~Request();
};

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
    this->header["Host"] = "";
    this->header["Transfer-Encoding"] = "";
    this->header["User-Agent"] = "";
}

bool    Request::findHeader(std::string key)
{
    if (header.count(key))
        return true;
}

bool    Request::setHeader(std::string key, std::string value)
{
    header[key] = value;
}

Request::~Request()
{
}

#endif //REQUEST_HPP