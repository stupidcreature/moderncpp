#include "gtest/gtest.h"

TEST(DummyTest, AlwaysPasses) {
    EXPECT_EQ(true, 1==1);
}

TEST(DummyTest, AlwaysFails) {
    EXPECT_NE(true, 1==1);
}

int main(int argc, char*argv[])
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

