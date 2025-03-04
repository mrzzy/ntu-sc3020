/*
 * SC3020
 * Project 1
 * Spy Store
 */

#include "spy_store.h"

#include "btree_node.h"
#include "mem_store.h"
#include <gtest/gtest.h>
#include <memory>

TEST(SpyStoreTest, InsertAndRetrieveBlock) {
  SpyStore store(std::make_unique<MemStore>());

  std::shared_ptr block = std::make_shared<BTreeNode>();
  BlockID id = store.insert(block);
  EXPECT_EQ(store.counts[SpyOpWrite][block->block_kind()], 1);
  store.update(id, block);
  EXPECT_EQ(store.counts[SpyOpWrite][block->block_kind()], 2);
  store.get_block(id);
  EXPECT_EQ(store.counts[SpyOpRead][block->block_kind()], 1);
}
