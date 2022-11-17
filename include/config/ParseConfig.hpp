#ifndef PARSECONFIG_HPP
#define PARSECONFIG_HPP

#include <string>
#include <fstream>

namespace webserv {

class ParseConfig
{
    public:
        ParseConfig(const std::string& configFilePath);
        ~ParseConfig();

    private:
        ParseConfig();
        ParseConfig(const ParseConfig& src);

        ParseConfig&	operator=(const ParseConfig& rhs);

        std::string						_configFilePath;
        std::ifstream					_configFile;

};

}	// namespace webserv

#endif	// PARSECONFIG_HPP