#include "gtest/gtest.h"
#include "util.h"
#include <vector>

using namespace std;

TEST(TestCases, SanityTest) {
	
	//Multiple
	for (int i = 0; i < 50; i++) {
		vector<int> v;
		v.push_back(1);
		v.push_back(2);
		v.push_back(3);

		auto a = UTIL_SelectRandom(v.begin(), v.end());

		ASSERT_NE(a, v.end());
		EXPECT_TRUE((*a >= 1) && (*a <= 3));
		cout << "Loop " << i << " selected " << *a << endl;
	}

	//One
	{
		vector<int> v2;
		v2.push_back(1);

		auto b = UTIL_SelectRandom(v2.begin(), v2.end());

		ASSERT_NE(b, v2.end());
		EXPECT_EQ(*b, 1);
		cout << "One" << endl;
	}

	//Zero
	{
		vector<int> v3;
		auto c = UTIL_SelectRandom(v3.begin(), v3.end());

		EXPECT_EQ(c, v3.end());
		cout << "Zero" << endl;
	}
}

int main(int argc, char** argv) {
	testing::InitGoogleTest(&argc, argv);
	RUN_ALL_TESTS();
	//cin.get();
}