//
// Created by zhuchaodi on 2025/3/4.
//

#ifndef BPTREE3_BPLUSTREE_H
#define BPTREE3_BPLUSTREE_H

#include "def.h"
#include "DiskManager.h"
class BPlusTree {
private:
    Node *root = nullptr;
    PageID root_page_id_;
    int min_degree_;

    DiskManager disk_manager_;
    void insertNonFullInternal(InternalNode *node, NodeKey key, const NodeValue &value);

    void insertNonFullLeaf(LeafNode *node, NodeKey key, const NodeValue &value);

    void splitLeafNode(InternalNode *parent, int index, LeafNode *child);

    void splitInternalNode(InternalNode *parent, int index, InternalNode *child);

    void bulkLoad(const std::vector<LeafNode> &leafNodes);

    // 查找键值对
    bool search(Node *node, NodeKey key, NodeValue &out);
    // 查找范围内的所有键值对
//    std::vector<NodeValue> rangeSearch(PageID nodePageId, NodeKey startKey, NodeKey endKey);
    std::vector<NodeValue> rangeSearch(Node *node, NodeKey startKey, NodeKey endKey, int &accessIndexTimesOut,
                                       int &accessBlockTimesOut);

    // 找到起始范围的叶子节点
    PageID findLeafPageId(PageID nodePageId, NodeKey key);
    std::tuple<PageID, int> findLeafNode(Node *node, NodeKey key, int accessIndexTimes);

public:
    BPlusTree(int degree, const char *ctx_path, const char * db_path, bool is_trunc);
//    ~BPlusTree();
    //获取记录个数
    int getPageNums();
    int getRecordNums(NodeKey start, NodeKey end);
    // 插入键值对
    void insert(NodeKey key, const NodeValue& value);

    void insertInternal(InternalNode *node, NodeKey key, const NodeValue &value);

    void insertLeaf(LeafNode *node, NodeKey key, const NodeValue &value);
    // 范围搜索
    std::vector<NodeValue>
    rangeSearch(NodeKey startKey, NodeKey endKey, int &accessIndexTimesOut, int &accessBlockTimesOut);

    // 打印树的内容（用于调试）
    void print(Node *node, int level);
    void print(PageID nodePageId, int level = 0);

    int getDepth();

    int getDepth(Node *node, int level = 0);

    bool search(NodeKey key, NodeValue &out, int &asscessTimesOut);
    // 打印树的内容
    void print();
};

#endif //BPTREE3_BPLUSTREE_H
