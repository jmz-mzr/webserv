#include "../include/http/HttpRequestHeader.hpp"

HttpRequestHeader::HttpRequestHeader(const std::string &str)
{
    buffer = str;
    this->parse();
}

std::string HttpRequestHeader::readLine()
{
    std::string line;
    size_t      i;

    if (index == std::string::npos)
        return "";
    i = buffer.find_first_of('\n', index);
    line = buffer.substr(index, i - index);
    if (i == std::string::npos)
        index = i;
    else
        index = i + 1;
    return line;
}

void        HttpRequestHeader::parseMethod(std::string line)
{
    size_t  i;
    std::string str;

    i = line.find_first_of('\n');
    str = str.substr(0, i);
    i = str.find_first_of(' ');

    method.assign(str, 0, i);
}

void        HttpRequestHeader::parse()
{
    std::string line;

    parseMethod(readLine());
    while((line = readLine()) != "")
    {
        //séparer la key et la value
        //créer un tableau key value avec des key prédéfinis
    }
}