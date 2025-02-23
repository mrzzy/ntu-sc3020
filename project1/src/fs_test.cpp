/*
 * SC3020
 * Project 1
 * Filesystem tests
*/

#include <gtest/gtest.h>
#include <iostream>
#include <ostream>
#include "fs.h"
TEST(fs_test, block_size) {
    std::cout << "block size: " << block_size() << std::endl;
}
