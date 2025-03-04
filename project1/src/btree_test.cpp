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
    std::shared_ptr<BTreeNode> node =
        std::dynamic_pointer_cast<BTreeNode>(store.get(i));
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
  std::shared_ptr<BTreeNode> node0 =
      std::dynamic_pointer_cast<BTreeNode>(store.get(0));
  // skip first key 0
  ASSERT_EQ(node0->keys[0], 1);
  ASSERT_EQ(node0->keys[node0->keys.size() - 1], split_at);
  std::shared_ptr<BTreeNode> node1 =
      std::dynamic_pointer_cast<BTreeNode>(store.get(1));
  // skip mid key propagated to parent
  ASSERT_EQ(node1->keys[0], split_at + 2);
  ASSERT_EQ(node1->keys[node1->keys.size() - 1], capacity - 1);

  // test recursive (repeated) propagation
  propagate = btree.load_internal(propagate);
  expected = std::map<Key, BlockID>{
      {0, 2},
  };
  ASSERT_EQ(propagate, expected);
  std::shared_ptr<BTreeNode> node2 =
      std::dynamic_pointer_cast<BTreeNode>(store.get(2));
  // check mid key propagated to parent
  ASSERT_EQ(node2->keys[0], split_at + 1);
}

TEST(btree_test, test_bulk_load) {
  // // test empty load
  std::map<Key, BlockID> key_pointers;
  uint16_t capacity = BTreeNode::fs_capacity();
  // MemStore store;
  // BTree btree(store);
  // ASSERT_EQ(btree.bulk_load(key_pointers), 0);
  // 
  // // test 1 level tree:
  // key_pointers.clear();
  // key_pointers[1] = 1;
  // MemStore store1;
  // BTree btree1(store1);
  // ASSERT_EQ(btree.bulk_load(key_pointers), 1);
  //
  // // test 2 level tree:
  // // root - leaf
  // key_pointers.clear();
  // for (int i = 0; i < capacity + 1; i++) {
  //   key_pointers[i] = i;
  // }
  // MemStore store2;
  // BTree btree2(store2);
  // ASSERT_EQ(btree2.bulk_load(key_pointers), 2);
  //
  // test 3 level tree:
  // root - internal - leaf
  key_pointers.clear();
  for (int i = 0; i < capacity * capacity * 2; i++) {
    key_pointers[i] = i;
  }
  MemStore store3;
  BTree btree3(store3);
  ASSERT_EQ(btree3.bulk_load(key_pointers), 3);

  // TODO(mrzzy):
}
