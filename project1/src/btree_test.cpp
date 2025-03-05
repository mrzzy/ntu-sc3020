/*
 * SC3020
 * Project 1
 * BTree Tests
 */

#include <cmath>
#include <cstdint>
#include <gtest/gtest.h>
#include <map>
#include <memory>
#include <unordered_set>

#include "btree.h"
#include "btree_node.h"
#include "id.h"
#include "mem_store.h"

TEST(btree_test, test_load_leaf) {
  // generate at least enough key pointer pairs to cause leaf node split
  std::map<Key, BlockID> key_pointers;
  uint16_t capacity = BTreeNode::fs_capacity();
  for (int i = capacity * 3 - 1; i >= 0; i--) {
    key_pointers[i] = i;
  }
  MemStore store;
  BTree btree(store);
  std::map<Key, BlockID> propagate = btree.load_leaf(key_pointers);

  // check propgated keys & pointers
  auto expected = std::map<Key, BlockID>{
      {0, 0},
      {capacity, 1},
      {capacity * 2, 2},
  };
  ASSERT_EQ(propagate, expected);

  // check last pointers of every block
  for (int i = 0; i < 3; i++) {
    std::shared_ptr node = store.get<BTreeNode>(i);
    if (i < 2) {
      // last pointers should point to next block
      ASSERT_EQ(node->pointers[node->pointers.size() - 1], i + 1);
    } else {
      // last block last pointer should point to null
      ASSERT_EQ(node->pointers[node->pointers.size() - 1], BLOCK_NULL);
    }
  }
}

TEST(btree_test, test_load_internal) {
  // generate at least enough key pointer pairs to cause internal node split
  std::map<Key, BlockID> key_pointers;
  uint16_t capacity = BTreeNode::fs_capacity();
  uint16_t split_at = std::ceil(capacity / 2.0);
  for (int i = capacity - 1; i >= 0; i--) {
    key_pointers[i] = i;
  }
  MemStore store;
  BTree btree(store);
  std::map<Key, BlockID> propagate = btree.load_internal(key_pointers);

  // check propgated keys & pointers
  auto expected = std::map<Key, BlockID>{
      {0, 0},
      {split_at + 1, 1},
  };
  ASSERT_EQ(propagate, expected);

  // check key split
  std::shared_ptr node0 = store.get<BTreeNode>(0);
  // skip first key 0
  ASSERT_EQ(node0->keys[0], 1);
  ASSERT_EQ(node0->keys[node0->keys.size() - 1], split_at);
  std::shared_ptr node1 = store.get<BTreeNode>(1);
  // skip mid key propagated to parent
  ASSERT_EQ(node1->keys[0], split_at + 2);
  ASSERT_EQ(node1->keys[node1->keys.size() - 1], capacity - 1);

  // test recursive (repeated) propagation
  propagate = btree.load_internal(propagate);
  expected = std::map<Key, BlockID>{
      {0, 2},
  };
  ASSERT_EQ(propagate, expected);
  std::shared_ptr<BTreeNode> node2 = store.get<BTreeNode>(2);
  // check mid key propagated to parent
  ASSERT_EQ(node2->keys[0], split_at + 1);
}

TEST(btree_test, test_bulk_load_get_range) {
  // test empty load
  std::map<Key, BlockID> key_pointers;
  uint16_t capacity = 3;
  MemStore store;
  BTree btree(store, capacity);
  ASSERT_EQ(btree.bulk_load(key_pointers), 0);
  ASSERT_EQ(btree.get(1), BLOCK_NULL);
  ASSERT_EQ(btree.range(0, 9), std::vector<BlockID>{});

  // test 1 level tree:
  key_pointers.clear();
  key_pointers[1] = 1;
  MemStore store1;
  BTree btree1(store1, capacity);
  ASSERT_EQ(btree1.bulk_load(key_pointers), 1);
  ASSERT_EQ(btree1.get(1), 1);
  ASSERT_EQ(btree1.range(0, 9), std::vector<BlockID>{1});
  ASSERT_EQ(btree1.range(0, 0), std::vector<BlockID>{});
  ASSERT_EQ(btree1.range(1, 9), std::vector<BlockID>{1});

  // test 2 level tree:
  // root - leaf
  key_pointers.clear();
  for (int i = 0; i < capacity + 1; i++) {
    key_pointers[i] = i;
  }
  MemStore store2;
  BTree btree2(store2, capacity);
  ASSERT_EQ(btree2.bulk_load(key_pointers), 2);

  std::vector<BlockID> ids2 = btree2.range(0, capacity);
  ASSERT_EQ(ids2.size(), capacity + 1);
  std::unordered_set<BlockID> ids_set2(ids2.begin(), ids2.end());

  for (int i = 0; i < capacity + 1; i++) {
    ASSERT_EQ(btree2.get(i), i);
    ASSERT_EQ(ids_set2.count(i), 1);
  }

  // test 3 level tree:
  // root - internal - leaf
  key_pointers.clear();
  for (int i = 0; i < 15; i++) {
    key_pointers[i] = i;
  }
  MemStore store3;
  BTree btree3(store3, capacity);
  ASSERT_EQ(btree3.bulk_load(key_pointers), 3);
  
  std::vector<BlockID> ids3 = btree3.range(0, 30);
  ASSERT_EQ(ids3.size(), 15);
  std::unordered_set<BlockID> ids_set3(ids3.begin(), ids3.end());

  for (int i = 0; i < 15; i++) {
    ASSERT_EQ(btree3.get(i), i);
    ASSERT_EQ(ids_set3.count(i), 1);
  }

  // test missing key
  ASSERT_EQ(btree3.get(999), BLOCK_NULL);
}
