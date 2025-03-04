/*
 * SC3020
 * Project 1
 * Metadata Block Tests
 */

#include "block.h"
#include "metadata.h"
#include <gtest/gtest.h>
TEST(metadata_test, test_write_read) {
  Metadata metadata;
  metadata.btree_root_id = 2;
  metadata.get_ids(BlockKindData).push_back(1);
  metadata.get_ids(BlockKindBTreeNode).push_back(1);
  ASSERT_THROW(metadata.get_ids(BlockKindMetadata), std::runtime_error);

  // test write
  std::stringstream ss;
  metadata.write(ss);
  std::cout << "wrote block size:" << ss.tellp() << std::endl;

  // test read from write
  Metadata read;
  read.read(ss);
  ASSERT_EQ(metadata, read);
}
