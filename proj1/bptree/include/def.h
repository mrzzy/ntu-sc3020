//
// Created on 2025/2/20.
//

#ifndef BPTREE_DEF_H
#define BPTREE_DEF_H
#include <iostream>
#include "fstream"
#include "string"
#include "TeamsRecord.h"
const int PAGE_SIZE = 1024; // 每一页大小设置为1024，用于存放每个node数据
const int DEGREE = 3; //定义3阶B+树，最小键值数是2，最大是2*3-1=5
using PageID = int;

#ifdef GOOGLE_TEST
using NodeKey = int;
struct Values {
    char data[12];  // 定长字符数组
    Values() {
        data[0] = 0;
    }
    Values(const char *str) {
        std::strncpy(this->data, str, sizeof(this->data));
        this->data[sizeof(this->data) - 1] = '\0'; // 确保末尾有空字符
    }
    // 重载赋值运算符
    Values& operator=(const Values& other) {
        if (this != &other) {
            // 使用 std::strcpy 来安全地复制字符数组内容
            std::strncpy(this->data, other.data, sizeof(this->data));
            this->data[sizeof(this->data) - 1] = '\0'; // 确保末尾有空字符
        }
        return *this;
    }
    bool operator==(const char* str) const {
        return std::strcmp(data, str) == 0;
    }
    // 输出函数用于调试
    void print() const {
        std::cout << "data: " << data << std::endl;
    }
};
using NodeValue=Values;
#else
using NodeValue=TeamsRecord;
using NodeKey = RecordKey;
std::ostream& operator<<(std::ostream& os, const RecordKey &key);

std::ostream& operator<<(std::ostream& os, const TeamsRecord &record);
#endif

struct Node {
    bool isLeaf; // 是否为叶子节点
    int nums;  //用于表示keys以及values的数目（B+树这个值永远相等）
    int child_nums; //用于表示子节点的个数
    PageID pageId; //自身的所在位置
    NodeKey keys[DEGREE * 2]; // 存储节点的键
    NodeValue values[DEGREE * 2]; // 存储叶子节点的值
    PageID children[DEGREE * 2 + 1]; // 存储子节点
    PageID next = -1; // 用于叶子节点之间的链表连接

    Node(bool leaf) : isLeaf(leaf), next(-1), nums(0), child_nums(0), pageId(-1) {}
};
#endif //BPTREE_DEF_H
