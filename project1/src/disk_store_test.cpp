/*
 * SC3020
 * Project 1
 * Disk Store Tests
 */

#include "btree_node.h"
#include "disk_store.h"
#include <gtest/gtest.h>
#include <memory>

std::filesystem::path path =
    std::filesystem::temp_directory_path() / "sc3020_disk_store";

TEST(disk_store_test, test_constructor) {
  DiskStore store(path);
  store.file.close();
  std::filesystem::remove(path);
}

TEST(disk_store_test, test_insert_get) {
  std::shared_ptr<Data> block1 = std::make_shared<Data>();
  std::shared_ptr<BTreeNode> block2 = std::make_shared<BTreeNode>();
  block2->insert(3, 3);

  DiskStore disk(path);
  BlockID id1 = disk.insert(block1);
  BlockID id2 = disk.insert(block2);

  EXPECT_EQ(id1, 0);
  EXPECT_EQ(id2, 1);

  std::shared_ptr<Metadata> meta = disk.get_meta();

  disk.persist();

  DiskStore disk2(path);

  EXPECT_EQ(*disk2.get<Data>(id1), *block1);
  EXPECT_EQ(*disk2.get<BTreeNode>(id2), *block2);
  EXPECT_THROW(disk2.get<Data>(999), std::runtime_error);

  // test update
  std::shared_ptr<BTreeNode> block3 =
      std::make_shared<BTreeNode>(BTreeNodeKindLeaf);
  block3->insert(1, 2);
  disk2.update(id2, block3);
  EXPECT_EQ(*disk2.get<BTreeNode>(id2), *block3);

  // check kind ids
  EXPECT_EQ(disk2.get_meta()->get_ids(BlockKindData)[0], 0);
  EXPECT_EQ(disk2.get_meta()->get_ids(BlockKindBTreeNode)[0], 1);
  std::filesystem::remove(path);
}
