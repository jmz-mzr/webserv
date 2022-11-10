#ifndef HTTPREQUESTHEADER
#define HTTPREQUESTHEADER

#include <string>

#include "Request.hpp"

class HttpRequestHeader
{

public:
    HttpRequestHeader();
    HttpRequestHeader(const std::string &str);
    HttpRequestHeader(const HttpRequestHeader& src);
    ~HttpRequestHeader();

    HttpRequestHeader&
    operator=(const HttpRequestHeader& rhs);
    void        parse(std::string str);
    void        setBuffer(std::string buf);

private:
    std::string buffer;
    std::string method;
    std::string path;
    size_t      index;
    Request     parsedRequest;
    Request     factory();
    void        parseMethod(std::string line);
    void        parsePath(std::string line);
    std::string readLine();
    std::string getValue(std::string line);
    std::string getKey(std::string line);
};
/*
HttpRequestHeader::HttpRequestHeader() { }

HttpRequestHeader::HttpRequestHeader(const HttpRequestHeader& src) { }


HttpRequestHeader&	HttpRequestHeader::operator=(const HttpRequestHeader& rhs)
{
    return *this;
}*/

#endif // HTTPREQUESTHEADER