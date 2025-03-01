//
// Created on 2025/2/20.
//

#ifndef BPLUSTREE_H
#define BPLUSTREE_H

#include <vector>
#include <algorithm>
#include "def.h"
#include "DiskManager.h"
class BPlusTree {
private:

    PageID root_page_id_;
    int min_degree_;
    DiskManager disk_manager_;
    // 插入到非叶子节点
    void insertNonFull(Node &node, NodeKey key, const NodeValue& value);

    // 分裂节点
    void splitNode(Node &parent, int index, Node &child);

    // 查找键值对
    NodeValue* search(PageID nodePageId, NodeKey key);

    // 查找范围内的所有键值对
    std::vector<NodeValue> rangeSearch(PageID nodePageId, NodeKey startKey, NodeKey endKey);

    // 找到起始范围的叶子节点
    PageID findLeafPageId(PageID nodePageId, NodeKey key);

public:
    BPlusTree(int degree, const char *ctx_path, const char * db_path, bool is_trunc);
    ~BPlusTree();
    //获取记录个数
    int getPageNums();
    int getRecordNums(NodeKey start, NodeKey end);
    // 插入键值对
    void insert(NodeKey key, const NodeValue& value);

    // 查找键值对
    NodeValue * search(NodeKey key);

    // 范围搜索
    std::vector<NodeValue > rangeSearch(NodeKey startKey, NodeKey endKey);

    // 打印树的内容（用于调试）
    void print(PageID nodePageId, int level = 0);

    int getDepth();

    int getDepth(PageID nodePageId, int level = 0);
    // 打印树的内容
    void print();
    //2025/3/1
    void bulkLoad(const std::vector<std::pair<NodeKey, NodeValue>>& sorted_data);
};



#endif //BPLUSTREE_H
