//
// Created by zuozhiyi on 2025/2/20.
//

#ifndef BPTREE_LOADCHECK_H
#define BPTREE_LOADCHECK_H

#include "gtest/gtest.h"

class LoadCheck :public ::testing::Test  {
protected:
    void SetUp() override;

    void TearDown() override;
};


#endif //BPTREE_LOADCHECK_H
