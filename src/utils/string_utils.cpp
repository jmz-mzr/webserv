# include <string>
# include <vector>

namespace webserv{

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
    return ret;
}

std::string    ft_string_remove(std::string src, char token)
{
    size_t j;

    for (size_t i = 0; src[i]; i++)
    {
        if (src[i] == token)
        {
            j = i;
            for (; src[j]; j++)
                src[j] = src[j + 1];
            src.resize(j);
        }
    }
    return src;
}

}
