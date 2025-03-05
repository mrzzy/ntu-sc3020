//
// Created on 2025/2/20.
//

#include "BPlusTree.h"
#include "queue"

//NodeValue *BPlusTree::search(PageID nodePageId, NodeKey key) {
//    int i = 0;
//    Node node(false);
//    disk_manager_.readPage(nodePageId, node);
//    while (i < node.nums && node.keys[i] < key) {
//        i++;
//    }
//    if (i < node.nums && node.keys[i] == key) {
//        if (node.isLeaf) {
//            return &node.values[i];
//        }
//    }
//    if (node.isLeaf) {
//        return nullptr;
//    }
//    return search(node.children[i], key);
//}
//
//std::vector<NodeValue> BPlusTree::rangeSearch(PageID nodePageId, NodeKey startKey, NodeKey endKey) {
//    std::vector<NodeValue> result;
//
//    PageID leafPageId = findLeafPageId(nodePageId, startKey);
//    Node leaf(false);
//    while (leafPageId != -1) {
//        disk_manager_.readPage(leafPageId, leaf);
//        for (int i = 0; i < leaf.nums; i++) {
//            if (leaf.keys[i] >= startKey && leaf.keys[i] <= endKey) {
//                result.push_back(leaf.values[i]);
//            } else if (leaf.keys[i] > endKey) {
//                return result;
//            }
//        }
//        leafPageId = leaf.next;
//
//    }
//    return result;
//}
//
//PageID BPlusTree::findLeafPageId(PageID nodePageId, NodeKey key) {
//    Node node(false);
//    disk_manager_.readPage(nodePageId, node);
//    if (node.isLeaf) {
//        return nodePageId;
//    }
//    int i = 0;
//    while (i < node.nums && node.keys[i] < key) {
//        i++;
//    }
//    return findLeafPageId(node.children[i], key);
//}

int BPlusTree::getPageNums() {

    return disk_manager_.getTotalPages();
}

int BPlusTree::getRecordNums(NodeKey start, NodeKey end) {
    int indexNums, blockNums;
    auto val = rangeSearch(start, end, indexNums, blockNums);
    return val.size();
}

BPlusTree::BPlusTree(int degree, const char *ctx_path, const char * db_path, bool is_trunc) : min_degree_(degree)
, disk_manager_(ctx_path, db_path, is_trunc)
{
    if (disk_manager_.isHasInitCtx()) {
        //如果已经存在了，那么直接load数据
        //todo bulk loading
        DataBaseContext ctx;
        disk_manager_.readCtx(ctx);
        root_page_id_ = ctx.rootPageId;
        std::cout << "init root page id: " << root_page_id_ << std::endl;
        std::vector<LeafNode> nodes;
        disk_manager_.readAllPage(nodes);
        bulkLoad(nodes);
    } else {
        DataBaseContext ctx;
        ctx.rootPageId = -1;
        ctx.keysSize = sizeof(NodeKey);
        ctx.pageSize = PAGE_SIZE;
        ctx.valuesSize = sizeof(NodeValue);
        disk_manager_.writeCtx(ctx);
        std::cout << "init database context success" << std::endl;
    }

}

void BPlusTree::insert(NodeKey key, const NodeValue& value) {
    if (root == nullptr) {
        PageID pageId = disk_manager_.allocatePage();
        root = new LeafNode();
        LeafNode *leaf = new LeafNode();
        leaf->pageId = pageId;
        root = leaf;
        disk_manager_.writePage(pageId, *leaf);
    }

    if (root->isLeaf) {
        insertLeaf(reinterpret_cast<LeafNode*>(root), key, value);
    } else {
        insertInternal(reinterpret_cast<InternalNode*>(root), key, value);
    }
}

void BPlusTree::splitLeafNode(InternalNode *parent, int index, LeafNode *child) {
    LeafNode nc;
    PageID newChildPageId = disk_manager_.allocatePage();
    nc.pageId = newChildPageId;

    LeafNode *newChild = &nc;
    int mid = min_degree_ - 1;
    for (int i = parent->nums - 1; i >= index; i--) {
        parent->keys[i + 1] = parent->keys[i];
    }
    parent->keys[index] = child->keys[mid];
    parent->nums++;
    parent->type = 2;
    parent->leafChildren.insert(parent->leafChildren.begin() + index + 1, newChildPageId);

    // 将右半部分的键和子节点分到新的子节点
    for (int i = mid + 1; i < child->nums; i++) {
        newChild->keys[i - mid - 1] = child->keys[i];
        newChild->values[i - mid - 1] = child->values[i];
        newChild->nums++;
    }
    child->nums = mid + 1;

    newChild->next = child->next;

    disk_manager_.writePage(newChild->pageId, *newChild);

    child->next = newChildPageId;
    disk_manager_.writePage(child->pageId, *child);

}

void BPlusTree::splitInternalNode(InternalNode *parent, int index, InternalNode *child) {
    InternalNode *newChild = new InternalNode();
    int mid = min_degree_ - 1;
    for (int i = parent->nums - 1; i >= index; i--) {
        parent->keys[i + 1] = parent->keys[i];
    }
    parent->keys[index] = child->keys[mid];
    parent->nums++;

    parent->internalChildren.insert(parent->internalChildren.begin() + index + 1, newChild);

    // 将右半部分的键和子节点分到新的子节点
    for (int i = mid + 1; i < child->nums; i++) {
        newChild->keys[i - mid - 1] = child->keys[i];
        newChild->nums++;
    }
    child->nums = mid + 1;

    newChild->type = child->type;
    if (newChild->type == 2) {
        //叶子节点
        newChild->leafChildren.assign(child->leafChildren.begin() + mid + 1, child->leafChildren.end());
        child->leafChildren.resize(mid + 1);
    } else {
        newChild->internalChildren.assign(child->internalChildren.begin() + mid + 1, child->internalChildren.end());
        child->internalChildren.resize(mid + 1);
    }


}

void BPlusTree::insertNonFullInternal(InternalNode *node, NodeKey key, const NodeValue &value) {
    int i = node->nums - 1;
    //插入内部节点
    while (i >= 0 && node->keys[i] > key) {
        i--;
    }
    i++;
    if (node->type == 1) {
        //底层都是内部节点
        InternalNode *child = node->internalChildren[i];
        if (child->nums == 2 * min_degree_ - 1) {
            splitInternalNode(node, i, reinterpret_cast<InternalNode*>(child));
            if (key > node->keys[i]) {
                child = node->internalChildren[i + 1];
            }
        }
        insertNonFullInternal(reinterpret_cast<InternalNode*>(child), key, value);
    } else {
        //底层都是叶子节点
        PageID childPageId = node->leafChildren[i];
        LeafNode child;
        disk_manager_.readPage(childPageId, child);

        if (child.nums == 2 * min_degree_ - 1) {
            splitLeafNode(node, i, &child);
            if (key > node->keys[i]) {
                childPageId = node->leafChildren[i + 1];
                disk_manager_.readPage(childPageId, child);
            }
        }
        insertNonFullLeaf(&child, key, value);
    }

}

void BPlusTree::insertNonFullLeaf(LeafNode *node, NodeKey key, const NodeValue &value) {
    int i = node->nums - 1;
    //插入内部节点
    // 插入到叶子节点
    // 找到插入位置
    while (i >= 0 && node->keys[i] > key) {
        node->keys[i + 1] = node->keys[i];
        node->values[i + 1] = node->values[i];
        i--;
    }
    node->keys[i + 1] = key;
    node->values[i + 1] = value;
    node->nums++;
    disk_manager_.writePage(node->pageId, *node);
}

//NodeValue *BPlusTree::search(NodeKey key) {
//    return search(root_page_id_, key);
//}
//
//std::vector<NodeValue> BPlusTree::rangeSearch(NodeKey startKey, NodeKey endKey) {
//    return rangeSearch(root_page_id_, startKey, endKey);
//}

//void BPlusTree::print(PageID nodePageId, int level) {
//    Node node(false);
//    disk_manager_.readPage(nodePageId, node);
//    if (node.isLeaf) {
//        std::cout << std::string(level, ' ') << "Leaf: ";
//        for (int i = 0; i < node.nums; i++) {
//            std::cout << node.keys[i] << " ";
//        }
//        std::cout << std::endl;
//    } else {
//        std::cout << std::string(level, ' ') << "Internal: ";
//        for (int i = 0; i < node.nums; i++) {
//            std::cout << node.keys[i] << " ";
//        }
//        std::cout << std::endl;
//        for (int i = 0; i < node.child_nums; i++) {
//            print(node.children[i], level + 1);
//        }
//    }
//}
//
void BPlusTree::print() {
    print(root, 0);
}

void BPlusTree::print(Node *node, int level) {
    if (node != nullptr) {
        std::cout << "Level " << level << ": ";
        if (node->isLeaf) {
            LeafNode *n = reinterpret_cast<LeafNode*>(node);
            for (int i = 0; i < n->nums; ++i) {
                std::cout << node->keys[i] << " ";
            }
            std::cout << "(leaf) ";
        } else {
            InternalNode *n = reinterpret_cast<InternalNode*>(node);
            for (int i = 0; i < n->nums; ++i) {
                std::cout << node->keys[i] << " ";
            }
            std::cout << "(internal) ";

        }

        std::cout << std::endl;

        if (!node->isLeaf) {
            InternalNode *n = reinterpret_cast<InternalNode*>(node);
            for (Node* child : n->internalChildren) {
                print(child, level + 1);
            }
            for (PageID pageId : n->leafChildren) {
                LeafNode child;
                disk_manager_.readPage(pageId, child);
                print(&child, level + 1);
            }
        }
    }
}

std::vector<NodeValue>
BPlusTree::rangeSearch(NodeKey startKey, NodeKey endKey, int &accessIndexTimesOut, int &accessBlockTimesOut) {
    accessBlockTimesOut = accessIndexTimesOut = 0;
    return rangeSearch(root, startKey, endKey, accessIndexTimesOut, accessBlockTimesOut);
}

std::vector<NodeValue> BPlusTree::rangeSearch(Node *node, NodeKey startKey, NodeKey endKey, int &accessIndexTimesOut,
                                              int &accessBlockTimesOut) {
    std::vector<NodeValue> result;
    auto [leafPageId, times] = findLeafNode(node, startKey, 1);
    accessIndexTimesOut = times;
    while (leafPageId != -1) {
        LeafNode leaf;
        disk_manager_.readPage(leafPageId, leaf);
        ++accessBlockTimesOut;
        for (int i = 0; i < leaf.nums; ++i) {

            if (leaf.keys[i] >= startKey && leaf.keys[i] <= endKey) {
                result.emplace_back(leaf.values[i]);
            } else if (leaf.keys[i] > endKey) {
                return result;
            }
        }
        leafPageId = leaf.next;

    }
    return result;
}

std::tuple<PageID, int> BPlusTree::findLeafNode(Node *node, NodeKey key, int accessIndexTimes) {
    if (node->isLeaf) {
        return {node->pageId, accessIndexTimes};
    }
    InternalNode *internalNode = reinterpret_cast<InternalNode*>(node);
    int i = 0;
    while (i < internalNode->nums && internalNode->keys[i] < key) {
        i++;
    }
    if (internalNode->type == 2) {
        PageID pageId = internalNode->leafChildren[i];
        LeafNode child;
        disk_manager_.readPage(pageId, child);
        //只统计内部节点的访问次数
        return findLeafNode(&child, key, accessIndexTimes);
    } else {
        return findLeafNode(internalNode->internalChildren[i], key, accessIndexTimes + 1);
    }
}

void BPlusTree::insertInternal(InternalNode *n, NodeKey key, const NodeValue &value) {
    if (n->nums == 2 * min_degree_ - 1) {
        InternalNode *newRoot = new InternalNode();
        newRoot->type = root->isLeaf ? 2 : 1;
        if (newRoot->type == 2) {
            newRoot->leafChildren.push_back(n->pageId);
        } else {
            newRoot->internalChildren.push_back(reinterpret_cast<InternalNode*>(n));
        }
        splitInternalNode(newRoot, 0, n);
        root = newRoot;
    }
    insertNonFullInternal(reinterpret_cast<InternalNode*>(root), key, value);
}

void BPlusTree::insertLeaf(LeafNode *n, NodeKey key, const NodeValue &value) {
    if (n->nums == 2 * min_degree_ - 1) {
        InternalNode *newRoot = new InternalNode();
        newRoot->type = 2;
        newRoot->leafChildren.push_back(n->pageId);
        splitLeafNode(newRoot, 0, n);
        root = newRoot;
        insertNonFullInternal(reinterpret_cast<InternalNode*>(root), key, value);
    } else {
        insertNonFullLeaf(reinterpret_cast<LeafNode*>(root), key, value);
    }
}

void BPlusTree::bulkLoad(const std::vector<LeafNode> &leafNodes) {
    if (leafNodes.empty()) {
        return ; //没有数据直接返回
    }

    // 生成叶子节点上的第一层内部节点
    std::vector<InternalNode*> currentLevel;
    InternalNode* internalNode = new InternalNode();
    internalNode->type = 2; // 指向叶子节点

    for (size_t i = 0; i < leafNodes.size(); ++i) {
        if (internalNode->nums == 2 * min_degree_ - 1) {
            // split 内部节点
            InternalNode *split = new InternalNode();
            split->type = internalNode->type;
            int mid = min_degree_ - 1;
            // 将右半部分的键和子节点分到新的子节点
            for (int j = mid + 1; j < internalNode->nums; j++) {
                split->keys[j - mid - 1] = internalNode->keys[j];
                split->nums++;
                internalNode->nums--;
            }
            split->leafChildren.assign(internalNode->leafChildren.begin() + mid + 1, internalNode->leafChildren.end());
            internalNode->leafChildren.resize(mid + 1);

            // 当前内部节点已满，生成新的内部节点
            currentLevel.push_back(internalNode);
            internalNode = split;
            internalNode->type = 2;
        }
        int index = leafNodes[i].nums - 1;
        internalNode->keys[internalNode->nums] = leafNodes[i].keys[index]; // 提取最后一个当做键值
        internalNode->leafChildren.push_back(leafNodes[i].pageId); // 指向叶子节点
        internalNode->nums++;
    }
    if (internalNode->nums == internalNode->leafChildren.size() && internalNode->nums > 0) {
        internalNode->nums--;
        internalNode->keys[internalNode->nums] = NodeKey (); // 填充空键值
    }
    if (internalNode->nums > 0) {
        currentLevel.push_back(internalNode); // 添加最后一个内部节点
    }

    // Step 2: 递归生成更高层的内部节点
    while (currentLevel.size() > 1) {
        std::vector<InternalNode*> nextLevel;
        InternalNode* parentNode = new InternalNode();
        parentNode->type = 1; // 指向内部节点

        for (size_t i = 0; i < currentLevel.size(); ++i) {
            if (parentNode->nums == 2 * min_degree_ - 1) {
                // split 内部节点
                InternalNode *split = new InternalNode();
                split->type = parentNode->type;
                int mid = min_degree_ - 1;
                // 将右半部分的键和子节点分到新的子节点
                for (int j = mid + 1; j < parentNode->nums; j++) {
                    split->keys[j - mid - 1] = parentNode->keys[j];
                    split->nums++;
                    parentNode->nums--;
                }

                if (split->type == 1) {
                    split->internalChildren.assign(parentNode->internalChildren.begin() + mid + 1, parentNode->internalChildren.end());
                    parentNode->internalChildren.resize(mid + 1);
                } else {
                    split->leafChildren.assign(parentNode->leafChildren.begin() + mid + 1, parentNode->leafChildren.end());
                    parentNode->leafChildren.resize(mid + 1);
                }


                // 当前父节点已满，生成新的父节点
                nextLevel.push_back(parentNode);
                parentNode = split;
                parentNode->type = 1;
            }
            int index = currentLevel[i]->nums - 1;
            parentNode->keys[parentNode->nums] = currentLevel[i]->keys[index]; // 提取第一个键值
            parentNode->internalChildren.push_back(currentLevel[i]); // 指向子节点
            parentNode->nums++;
        }
        if (parentNode->nums == parentNode->internalChildren.size() && parentNode->nums > 0) {
            parentNode->nums--;
            parentNode->keys[parentNode->nums] = NodeKey (); // 填充空键值
        }
        if (parentNode->nums > 0) {
            nextLevel.push_back(parentNode); // 添加最后一个父节点
        }
        currentLevel = nextLevel;
    }

    // Step 3: 设置根节点
    root = currentLevel[0];
}

bool BPlusTree::search(NodeKey key, NodeValue &out, int &asscessTimesOut) {
    return search(root, key, out);
}


bool BPlusTree::search(Node *node, NodeKey key, NodeValue &out) {
    int i = 0;
    while (i < node->nums && node->keys[i] < key) {
        i++;
    }
    if (i < node->nums && node->keys[i] == key) {
        if (node->isLeaf) {
            out = reinterpret_cast<LeafNode*>(node)->values[i];
            return true;
        }
    }
    if (node->isLeaf) {
        return false;
    }
    InternalNode *n = reinterpret_cast<InternalNode*>(node);
    if (n->type == 2) {
        LeafNode child;
        disk_manager_.readPage(n->leafChildren[i], child);
        return search(&child, key, out);
    } else {
        return search(n->internalChildren[i], key, out);
    }
}
//
int BPlusTree::getDepth() {
    return getDepth(root, 1);
}

int BPlusTree::getDepth(Node *node, int level) {
    if (!node) {
        return level;
    }
    if (node->isLeaf) {
        return level + 1;
    }
    InternalNode *n = reinterpret_cast<InternalNode*>(node);
    if (n->type == 1) {
        return getDepth(n->internalChildren[0], level + 1);
    } else {
        return level + 1;
    }
    //使用bfs进行depth搜索
//    std::queue<std::tuple<InternalNode, int>> q;
//
//    q.push({*reinterpret_cast<InternalNode*>(node), 1});
//    int maxLevel = 0;
//    while (!q.empty()) {
//        InternalNode n;
//        int l;
//        std::tie(n, l) = q.front(); q.pop();
//        maxLevel = std::max(maxLevel, l);
//        if (!n.isLeaf) {
//            InternalNode *internalNode = reinterpret_cast<InternalNode*>(&n);
//            if (internalNode->type == 2) {
//                maxLevel = std::max(maxLevel, l + 1);
//            } else {
//                for (auto & i : internalNode->internalChildren) {
//                    q.push({*i, l + 1});
//                }
//            }
//
//        } else {
//            maxLevel = std::max(maxLevel, l + 1);
//        }
//    }
//    return maxLevel;
}
//
//int BPlusTree::getDepth(PageID nodePageId, int level) {
//    //使用bfs进行depth搜索
//    std::queue<std::tuple<PageID, int>> q;
//
//    q.push({nodePageId, 1});
//    Node node(false);
//    int maxLevel = 0;
//    while (!q.empty()) {
//        auto [pageId, l] = q.front(); q.pop();
//        maxLevel = std::max(maxLevel, l);
//        disk_manager_.readPage(pageId, node);
//        for (int i = 0; i < node.child_nums; ++i) {
//            q.push({node.children[i], l + 1});
//        }
//    }
//    return maxLevel;
//}
//
//BPlusTree::~BPlusTree() {
//    DataBaseContext ctx;
//    disk_manager_.readCtx(ctx);
//    ctx.rootPageId = root_page_id_;
//    disk_manager_.writeCtx(ctx);
//    std::cout << "flush context success, current root_page_id=" << root_page_id_ << std::endl;
//}
