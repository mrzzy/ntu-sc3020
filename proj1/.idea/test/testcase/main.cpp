#include "gtest/gtest.h"
#include <cstdlib>
const char * recovery = "testcase";

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    int ret = RUN_ALL_TESTS();
    return ret;
}
