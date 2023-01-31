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
	"lexer06",
	"errorpage00",
	"errorpage01",
	"errorpage02",
	"errorpage03",
	"errorpage04",
	"maxbodysize00",
	"maxbodysize01",
	"maxbodysize02",
	"maxbodysize03",
	"maxbodysize04",
	"maxbodysize05",
	"limitexcept00",
	"limitexcept01",
	"limitexcept02",
	"return00",
	"return01",
	"return02",
	"return03",
	"return04",
	"return05"
};

TEST_P(ConfigParsing, SyntaxErrorTest) {
	ConfigParser	test(filePath);
	ASSERT_THROW(test.parseFile(), SyntaxErrorException);
}

INSTANTIATE_TEST_SUITE_P(, ConfigParsing, testing::ValuesIn(files));