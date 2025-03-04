/*
 * SC3020
 * Project 1
 * Memory Store Tests
 */

#include "block.h"
#include "btree.h"
#include "data.h"
#include "mem_store.h"
#include <gtest/gtest.h>
#include <memory>

TEST(mem_store_test, test_insert_get_kind_ids) {
  std::shared_ptr<Data> block1 = std::make_shared<Data>();
  std::shared_ptr<BTreeNode> block2 = std::make_shared<BTreeNode>();

  MemStore ms;
  Store &store = ms;
  BlockID id1 = store.insert(block1);
  BlockID id2 = store.insert(block2);

  EXPECT_EQ(id1, 0);
  EXPECT_EQ(id2, 1);

  EXPECT_EQ(store.get<Data>(id1), block1);
  EXPECT_EQ(store.get<BTreeNode>(id2), block2);
  EXPECT_THROW(store.get<Block>(999), std::runtime_error);

  // test update
  std::shared_ptr<BTreeNode> block3 =
      std::make_shared<BTreeNode>(BTreeNodeKindLeaf);
  block3->insert(1, 2);
  store.update(id2, block3);
  EXPECT_EQ(store.get<BTreeNode>(id2), block3);

  // check kind ids
  EXPECT_EQ(store.kind_ids(BlockKindData)[0], 0);
  EXPECT_EQ(store.kind_ids(BlockKindBTreeNode)[0], 1);
}
