#ifndef REQUEST_HPP
#define REQUEST_HPP

#include <map>
#include <iostream>
class Request
{
private:
    void    initHeader();
    std::string path;
    std::string method;
    int         code;
    std::map<std::string, std::string> header;
public:
    Request(/* args */);
    ~Request();
    Request(const Request& src);

    bool    findHeader(std::string key);
    void    setHeader(std::string key, std::string value);    
    void    setPath(std::string str);
    void    printHeader();
    void    setCode(int i);
    int     getCode();
    Request&
    operator=(const Request& rhs);
};

#endif //REQUEST_HPP