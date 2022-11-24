#include "utils/utils.hpp"

#include <algorithm>
#include <string>
#include <vector>

#include "doctest/doctest.h"

using namespace webserv;

TEST_SUITE("trim functions") {

	struct test_t {
		std::string name;
		std::string	toTrim;
		std::string	result;
	};

	void	doChecks(test_t* list, std::string (*func)(const std::string&))
	{
		for (int i = 0; i < 5; i++) {
			SUBCASE((list + i)->name.c_str()) {
				CHECK_EQ(func((list + i)->toTrim), (list + i)->result);
			}
		}
	}

	TEST_CASE("trim_left()") {
		test_t list[5] = {
			{ "empty", " \t \n \r \f \v", "" },
			{ "spaces left", "   \t \n \r \f \vtest", "test" },
			{ "spaces right", "test  \r\r\r \f\v", "test  \r\r\r \f\v" },
			{ "spaces both", " \t \ntest \r \f \v", "test \r \f \v" },
			{ "spaces between", " \t \ntest \ttest\v", "test \ttest\v" }
		};
		doChecks(list, trim_left);
	}

	TEST_CASE("trim_right()") {
		test_t list[5] = {
			{ "empty", " \t \n \r \f \v", "" },
			{ "spaces left", "   \t \n \r \f \vtest", "   \t \n \r \f \vtest" },
			{ "spaces right", "test  \r\r\r \f\v", "test" },
			{ "spaces both", " \t \ntest \r \f \v", " \t \ntest" },
			{ "spaces between", " \t \ntest \ttest\v", " \t \ntest \ttest" }
		};
		doChecks(list, trim_right);
	}

	TEST_CASE("trim()") {
		test_t list[5] = {
			{ "empty", " \t \n \r \f \v", "" },
			{ "spaces left", "   \t \n \r \f \vtest", "test" },
			{ "spaces right", "test  \r\r\r \f\v", "test" },
			{ "spaces both", " \t \ntest \r \f \v", "test" },
			{ "spaces between", " \t \ntest \ttest\v", "test \ttest" }
		};
		doChecks(list, trim);
	}

}