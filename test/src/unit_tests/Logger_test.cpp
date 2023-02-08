#include "utils/Logger.hpp"

#include <gtest/gtest.h>

#include "webserv_config.hpp"

using namespace webserv;

class LoggerClass : public testing::Test {
protected:
	Logger &L = Logger::getInstance();
};

TEST_F(LoggerClass, getInstanceFn)
{
	Logger &L1 = Logger::getInstance();

	ASSERT_EQ(&L, &L1);
}

TEST_F(LoggerClass, OutputStream)
{
	if (LOG_OSTREAM & Logger::kFile) {
		const std::ofstream& fstream = L.getLogfile();
		EXPECT_TRUE(fstream.is_open());
	}
	else {
		SUCCEED();
	}
}