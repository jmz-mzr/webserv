#include "utils/Logger.hpp"

#include "doctest/doctest.h"

#include "webserv_config.hpp"

using namespace webserv;

TEST_SUITE("Logger class") {

	Logger &L = Logger::getInstance();

	TEST_CASE("getInstance() function") {
		Logger	&L1 = Logger::getInstance();
		REQUIRE_EQ(&L1, &L);
	}

	TEST_CASE("Output channel") {
		if (CONF_LOG_OUT & kFile) {
			SUBCASE("file") {
				const std::ofstream& fstream = L.getLogfile();
				CHECK(fstream.is_open());
			}
		}
	}
}