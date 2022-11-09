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

private:
    std::string buffer;
    std::string method;
    size_t      index;
    Request     parsedRequest;
    Request     factory();
    void        parse();
    void        parseMethod(std::string line);
    std::string readLine();
};

HttpRequestHeader::HttpRequestHeader() { }

HttpRequestHeader::HttpRequestHeader(const HttpRequestHeader& src) { }

HttpRequestHeader::~HttpRequestHeader() { }

HttpRequestHeader&	HttpRequestHeader::operator=(const HttpRequestHeader& rhs)
{
    return *this;
}

#endif // HTTPREQUESTHEADER