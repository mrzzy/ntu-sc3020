//
// Created on 2025/2/20.
//

#include "InsertCheck.h"
#include "BPlusTree.h"
void InsertCheck::SetUp() {
    Test::SetUp();
}

void InsertCheck::TearDown() {
    Test::TearDown();
}


TEST_F(InsertCheck, tst_insert) {
    BPlusTree tree(DEGREE, "./test.ctx", "./a.db", true);

    tree.insert(6, "Six");
    tree.insert(7, "Seven");
    tree.insert(8, "Eight");
    tree.insert(9, "Nine");
    tree.insert(10, "Ten");
    tree.insert(11, "eleven");
    tree.insert(12, "twelve");
    tree.insert(13, "thirteen");
    tree.insert(14, "fourteen");
    tree.insert(15, "fifteen");
    tree.insert(1, "one");
    tree.insert(2, "two");
    tree.insert(3, "three");
    tree.insert(4, "four");
    tree.insert(5, "Five");
    NodeValue out;
    int times;
    EXPECT_TRUE(tree.search(6, out, times));
    EXPECT_EQ(out, "Six");
}