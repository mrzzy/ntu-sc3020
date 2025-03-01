//
// Created on 2025/2/20.
//

#ifndef BPTREE_DATABASE_CONTEXT_H
#define BPTREE_DATABASE_CONTEXT_H

#include "def.h"
struct DataBaseContext {
    PageID rootPageId;  //当前根节点所在位置
    int pageSize;       //每一页定义的大小
    int keysSize;       //存储的键值对size
    int valuesSize;     //存储的value值size
};
#endif //BPTREE_DATABASE_CONTEXT_H
