#include "config/ConfigParser.hpp"
#include "core/Webserv.hpp"
#include "utils/exceptions.hpp"
#include "utils/global_defs.hpp"

#include <gtest/gtest.h>

using namespace webserv;

TEST(ConfigParsingInit, ctorEmptyArgument) {
	ConfigParser test("");
	ASSERT_EQ(test.getFilePath(), XSTR(CONF_FILE));
}

TEST(ConfigParsingInit, emptyFile) {
	webserv::Webserv	webserv;
	int argc = 2;
	char empty[] = "";
	char file[] = "tests/cpp_gtest/data/empty";
	char *strs[2] = { empty, file };
	char **argv = strs;

	ASSERT_THROW(webserv.init(argc, argv), LogicErrorException);
}

TEST(ConfigParsingInit, emptyline) {
	ConfigParser test("tests/cpp_gtest/data/emptyline");
	ASSERT_NO_THROW(test.parseFile());
}

class ConfigParsing : public testing::TestWithParam<std::string> {
protected:
	void	SetUp() override {
		confDir = "tests/cpp_gtest/data/";
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
	// "errorpage00",
	"errorpage01",
	"errorpage02",
	"errorpage03",
	"errorpage04",
	// "maxbodysize00",
	"maxbodysize01",
	"maxbodysize02",
	"maxbodysize03",
	"maxbodysize04",
	"maxbodysize05",
	"limitexcept00",
	"limitexcept01",
	"limitexcept02",
	// "return00",
	"return01",
	"return02",
	"return03",
	"return04",
	"return05",
	"cgipass00",
	"cgipass01",
	"cgipass02",
	"listen00",
	"listen01",
	"listen02",
	"listen03",
	"listen04",
	"listen05",
	"listen06",
	"listen07",
	"location00",
	"location01",
	"location02",
	"location03",
	"location04",
	"location05"
};

TEST_P(ConfigParsing, SyntaxErrorTest) {
	ConfigParser	test(filePath);
	ASSERT_THROW(test.parseFile(), SyntaxErrorException);
}

INSTANTIATE_TEST_SUITE_P(, ConfigParsing, testing::ValuesIn(files));