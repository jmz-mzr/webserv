#include "config/ConfigParser.hpp"
#include "utils/exceptions.hpp"
#include "utils/global_defs.hpp"

#include <gtest/gtest.h>

using namespace webserv;

TEST(ConfigParsingInit, ctorEmptyArgument) {
	ConfigParser test("");
	ASSERT_EQ(test.getFilePath(), XSTR(CONF_FILE));
}

TEST(ConfigParsingInit, emptyFile) {
	ConfigParser test("test/data/bad_conf/empty");
	ASSERT_THROW(test.parseFile(), FatalErrorException);
}

class ConfigParsing : public testing::TestWithParam<std::string> {
protected:
	void	SetUp() override {
		confDir = "test/data/bad_conf/";
		filePath = confDir + GetParam();
	}

	std::string	confDir;
	std::string	filePath;
};

std::string files[] = {
	"lexer00",
	"lexer01",
	"lexer02",
	"lexer03",
	"lexer04",
	"lexer05",
	"lexer06"
};

TEST_P(ConfigParsing, SyntaxErrorTest) {
	ConfigParser	test(filePath);
	ASSERT_THROW(test.parseFile(), SyntaxErrorException);
}

INSTANTIATE_TEST_SUITE_P(, ConfigParsing, testing::ValuesIn(files));