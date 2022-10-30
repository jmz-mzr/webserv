#ifndef HTTPREQUESTHEADER
#define HTTPREQUESTHEADER

#include <string>

class HttpRequestHeader
{

public:
    HttpRequestHeader();
    HttpRequestHeader(const HttpRequestHeader& src);
    ~HttpRequestHeader();

    HttpRequestHeader&
    operator=(const HttpRequestHeader& rhs);

private:
    std::string buffer;

};

HttpRequestHeader::HttpRequestHeader() { }

HttpRequestHeader::HttpRequestHeader(const HttpRequestHeader& src) { }

HttpRequestHeader::~HttpRequestHeader() { }

HttpRequestHeader&	HttpRequestHeader::operator=(const HttpRequestHeader& rhs)
{
    return *this;
}

#endif // HTTPREQUESTHEADER