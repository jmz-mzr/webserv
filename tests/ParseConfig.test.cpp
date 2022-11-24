#include "config/ParseConfig.hpp"

#include <string>

#include "doctest/doctest.h"

#include "utils/utils.hpp"
#include "webserv_config.hpp"

using namespace webserv;

TEST_SUITE("ParseConfig class") {

	TEST_CASE("Default constructor") {
        SUBCASE("default") {
            ParseConfig test;
            CHECK_EQ(test.getConfigFilePath(), DEFAULT_CONF_FILE);
            CHECK(test.getConfigFile().is_open());
        }
        SUBCASE("with good config file") {
            std::string path("config/test.conf");
            ParseConfig test(path);
            CHECK_EQ(test.getConfigFilePath(), path);
            CHECK(test.getConfigFile().is_open());
        }
        SUBCASE("with bad config file") {
            std::string path("config/dummy.conf");
            ParseConfig test(path);
            CHECK_EQ(test.getConfigFilePath(), DEFAULT_CONF_FILE);
            CHECK(test.getConfigFile().is_open());
        }
        //! cannot undef DEFAULT_CONF_FILE to test constructor throw
	}

}