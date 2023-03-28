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
	char file[] = "tests/cpp_gtest/data/incorrect/emptyConf";
	char *strs[2] = { empty, file };
	char **argv = strs;

	ASSERT_THROW(webserv.init(argc, argv), LogicErrorException);
}

TEST(ConfigParsingInit, emptyLine) {
	ConfigParser test("tests/cpp_gtest/data/correct/emptyLine");
	ASSERT_NO_THROW(test.parseFile());
}

TEST(ConfigParsingInit, listenAllowed) {
	ConfigParser test("tests/cpp_gtest/data/correct/listenAllowed");
	ASSERT_NO_THROW(test.parseFile());
}

class ConfigParsing : public testing::TestWithParam<std::string> {
protected:
	void	SetUp() override {
		confDir = "tests/cpp_gtest/data/incorrect/";
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
	"maxbodysize06",
	"limitexcept00",
	"limitexcept01",
	"limitexcept02",
	"limitexcept03",
	"ignoreexcept00",
	"ignoreexcept01",
	"ignoreexcept02",
	"ignoreexcept03",
	// "return00",
	"return01",
	"return02",
	"return03",
	"return04",
	"return05",
	"root00",
	"root01",
	"root02",
	"root03",
	"alias00",
	"alias01",
	"alias02",
	"alias03",
	"autoindex00",
	"autoindex01",
	"hidelimitrule00",
	"hidelimitrule01",
	"hidedirectory00",
	"hidedirectory01",
	"cgipass00",
	"cgipass01",
	"cgipass02",
	"listen00",
	// "listen01",
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
	"location05",
	"location06"
};

TEST_P(ConfigParsing, SyntaxErrorTest) {
	ConfigParser	test(filePath);
	ASSERT_THROW(test.parseFile(), SyntaxErrorException);
}

INSTANTIATE_TEST_SUITE_P(, ConfigParsing, testing::ValuesIn(files));
