#include "gtest/gtest.h"
#include "SomeClassToTest.h"


TEST(DummyTest, AlwaysPasses) {
    EXPECT_EQ(true, 1==1);
}

TEST(DummyTest, AlwaysFails) {
    EXPECT_NE(true, 1==1);
}

struct TestFixture : testing::Test
{
    std::unique_ptr<SomeClassToTest> testclass;

    TestFixture() {
        testclass = std::make_unique<SomeClassToTest>();
    }
    virtual ~TestFixture() = default;
};


struct testdata {
    int64_t value1;
    int64_t value2;
    int64_t result;
    int64_t initial_state;
};

struct ParameterizedTest : TestFixture, testing::WithParamInterface<testdata>
{
    ParameterizedTest() {
        // initialize object
        testclass->some_internal_state_ = GetParam().initial_state;
    }
};

TEST_P(ParameterizedTest, TestUsingParameters) {
    auto params = GetParam();
    EXPECT_EQ(params.initial_state, testclass->some_internal_state_); // as initialized using testdata input
    EXPECT_EQ(params.result, testclass->AddTwoNumbers(params.value1, params.value2));
}

TEST_F(TestFixture, TestUsingTestFixture) {
    EXPECT_EQ(5, testclass->AddTwoNumbers(2,3));
}



INSTANTIATE_TEST_CASE_P(SomeArbitraryPrefix,
                        ParameterizedTest,
                        testing::Values(
                                 testdata{2,3,5,42}
                        )
                        );



//TEST_P(ParameterizedTestCases, DummyTest) {
//
//}
//
//testing::
//
//INSTANTIATE_TEST_CASE_P()

int main(int argc, char*argv[])
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

