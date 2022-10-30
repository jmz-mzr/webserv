class HttpRequestHeader
{

public:
    HttpRequestHeader();
    HttpRequestHeader(const HttpRequestHeader& src);
    ~HttpRequestHeader();

    HttpRequestHeader&  operator=(const HttpRequestHeader& rhs);

};

HttpRequestHeader::HttpRequestHeader() { }

HttpRequestHeader::HttpRequestHeader(const HttpRequestHeader& src) { }

HttpRequestHeader::~HttpRequestHeader() { }

HttpRequestHeader&	HttpRequestHeader::operator=(const HttpRequestHeader& rhs)
{
    return *this;
}