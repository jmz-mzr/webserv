#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <algorithm>

std::vector<std::string>    ft_string_split(std::string &str, std::string delim)
{
    std::string tmp;
    std::vector<std::string> ret;

    while (str.find(delim) != std::string::npos)
    {
        tmp = str.substr(0, str.find(delim));
        ret.push_back(tmp);
        str.erase(0, str.find(delim));
    }
}