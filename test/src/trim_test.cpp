#include "utils/utils.hpp"

#include <algorithm>
#include <array>
#include <string>
#include <tuple>

#include <gtest/gtest.h>

using namespace webserv;

enum class Side : size_t { left, right, both };

std::array<std::string, 6> testNames = {
	"only_spaces",
	"spaces_left",
	"spaces_right",
	"spaces_both",
	"spaces_between",
	"no_spaces"
};

std::array<std::string, 3>	trimFnNames = {
	"trimLeft",
	"trimRight",
	"trim"
};

class	TrimTest : public testing::TestWithParam<std::tuple<size_t, size_t> > {
public:
	typedef std::string (*trim_fn)(const std::string&);

	struct Data {
		trim_fn			fnPtr;
		std::string		fnName;
	};

	static std::array<Data, 3>							fnData;
	static std::array<std::string, 6>					strings;
	static std::array<std::array<std::string, 6>, 3>	results;

	static void SetUpTestSuite() {

		TrimTest::strings = {
			" \t \n \r \f \v",
			"   \t \n \r \f \v?<>@#",
			"test  \r\r\r \f\v",
			" \t \n42 \r \f \v",
			" \t \n[]\"\' \t\\{}+=-_\v",
			"\b\x0e\x1f\x7F"
		};

		TrimTest::results[size_t(Side::left)] = {
			"",
			"?<>@#",
			"test  \r\r\r \f\v",
			"42 \r \f \v",
			"[]\"\' \t\\{}+=-_\v",
			"\b\x0e\x1f\x7F"
		};

		TrimTest::results[size_t(Side::right)] = {
			"",
			"   \t \n \r \f \v?<>@#",
			"test",
			" \t \n42",
			" \t \n[]\"\' \t\\{}+=-_",
			"\b\x0e\x1f\x7F"
		};

		TrimTest::results[size_t(Side::both)] = {
			"",
			"?<>@#",
			"test",
			"42",
			"[]\"\' \t\\{}+=-_",
			"\b\x0e\x1f\x7F"
		};

		TrimTest::fnData[size_t(Side::left)] = { &trimLeft, "trimLeft()" };
		TrimTest::fnData[size_t(Side::right)] = { &trimRight, "trimRight()" };
		TrimTest::fnData[size_t(Side::both)] = { &trim, "trim()" };
	}

protected:

	trim_fn			fn;
	std::string		str;
	std::string		res;

	void SetUp() override {
		size_t	trimType = std::get<0>(GetParam());
		size_t	testIndex = std::get<1>(GetParam());

		fn = TrimTest::fnData[trimType].fnPtr;
		str = TrimTest::strings[testIndex];
		res = TrimTest::results[trimType][testIndex];
	}

};

std::array<TrimTest::Data, 3>				TrimTest::fnData;
std::array<std::string, 6>					TrimTest::strings;
std::array<std::array<std::string, 6>, 3>	TrimTest::results;

TEST_P(TrimTest, )
{
	EXPECT_EQ(fn(str), res);
}

INSTANTIATE_TEST_SUITE_P(
	, TrimTest,
	testing::Combine(
		testing::Values(Side::left, Side::right, Side::both),
		testing::Range(0UL, testNames.size(), 1)
	),
	[](const testing::TestParamInfo<TrimTest::ParamType>& inf) {
		std::string name = trimFnNames[std::get<0>(inf.param)]
							+ "_" + testNames[std::get<1>(inf.param)];
		return name;
});

int main(int argc, char **argv)
{
	testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
