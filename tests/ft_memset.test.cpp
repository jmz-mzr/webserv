#include "utils/utils.hpp"

#include <string>

#include "doctest/doctest.h"

using namespace webserv;

TEST_SUITE("ft_memset()") {

    TEST_CASE("copy") {
        char        s[10 + 1] = "..........";
        std::string str;
        std::string result;

        SUBCASE("start") {
            std::string result = "aaaaa.....";
            str.assign(reinterpret_cast<char *>(ft_memset(s, 'a', 5)));
            CHECK_EQ(str, result);
            CHECK_EQ(str.length(), 10);
        }

        SUBCASE("mid") {
            std::string result = "..bbb.....";
            ft_memset(s + 2, 'b', 3);
            str.assign(reinterpret_cast<char *>(s));
            CHECK_EQ(str, result);
            CHECK_EQ(str.length(), 10);
        }

        SUBCASE("end") {
            std::string result = ".........c";
            ft_memset(s + 9, 'c', 1);
            str.assign(reinterpret_cast<char *>(s));
            CHECK_EQ(str, result);
            CHECK_EQ(str.length(), 10);
        }
    }

    TEST_CASE("return value") {
        char s[10] = "";
        char *p = reinterpret_cast<char *>(ft_memset(s + 4, 0, 1));
        CHECK_EQ(p, s + 4);
    }
}