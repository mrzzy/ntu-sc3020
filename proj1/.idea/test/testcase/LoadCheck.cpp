//
// Created by zuozhiyi on 2025/2/20.
//

#include "LoadCheck.h"
#include "BPlusTree.h"
void LoadCheck::SetUp() {
    Test::SetUp();
}

void LoadCheck::TearDown() {
    Test::TearDown();
}

TEST_F(LoadCheck, tst_load_data) {
    {
        BPlusTree tree(DEGREE, "./test.ctx", "./a.db", true);

        // 插入键值对
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
    }

    {
        //测试重新load起来之后，能否正常进行search
        BPlusTree tree2(DEGREE, "./test.ctx", "./a.db", false);
        EXPECT_EQ(*tree2.search(6), "Six");

    }

}
