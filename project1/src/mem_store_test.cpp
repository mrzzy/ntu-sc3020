/*
 * SC3020
 * Project 1
 * Memory Store Tests
 */

#include "btree.h"
#include "data.h"
#include "mem_store.h"
#include <gtest/gtest.h>
#include <memory>

TEST(mem_store_test, test_insert_get) {
  std::shared_ptr<Data> block1 = std::make_shared<Data>();
  std::shared_ptr<BTreeNode> block2 = std::make_shared<BTreeNode>();

  MemStore ms;
  Store &store = ms;
  BlockID id1 = store.insert(block1);
  BlockID id2 = store.insert(block2);

  EXPECT_EQ(id1, 0);
  EXPECT_EQ(id2, 1);

  EXPECT_EQ(store.get(id1), block1);
  EXPECT_EQ(store.get(id2), block2);
  EXPECT_THROW(store.get(999), std::runtime_error);

  // test update
  std::shared_ptr<BTreeNode> block3 = std::make_shared<BTreeNode>(2, BTreeNodeKindLeaf);
  store.update(id2, block3);
  EXPECT_EQ(store.get(id2), block3);
}
