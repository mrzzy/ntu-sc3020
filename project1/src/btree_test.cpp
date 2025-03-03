/*
 * SC3020
 * Project 1
 * BTree Tests
 */

#include <gtest/gtest.h>
#include <map>
#include <memory>

#include "btree.h"
#include "btree_node.h"
#include "id.h"
#include "mem_store.h"

TEST(btree_test, test_load_leaf) {
  BTreeNode leaf(BTreeNodeKindLeaf);

  // generate at least enough key pointer pairs to cause leaf node split
  std::map<Key, BlockID> key_pointers;
  for (int i = 0; i < leaf.capacity * 3; i++) {
    key_pointers[i] = i;
  }
  MemStore store;
  BTree btree(store);
  std::map<Key, BlockID> propagate = btree.load_leaf(key_pointers);

  // check propgated keys & pointers
  auto expected = std::map<Key, BlockID>{
      {0, 0},
      {leaf.capacity, 1},
      {leaf.capacity * 2, 2},
  };
  ASSERT_EQ(propagate, expected);

  // check last pointers of every block
  for (int i = 0; i < 3; i ++ ){
    std::shared_ptr<BTreeNode> node = std::dynamic_pointer_cast<BTreeNode>(store.get(i));
    if(i < 2) {
      // last pointers should point to next block
      ASSERT_EQ(node->pointers[node->pointers.size() - 1], i+1);
    } else {
      // last block last pointer should point to null
      ASSERT_EQ(node->pointers[node->pointers.size() - 1], BLOCK_NULL);
    }
  }
}
