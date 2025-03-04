/*
 * SC3020
 * Project 1
 * Metadata Block Tests
 */


#include "metadata.h"
#include <gtest/gtest.h>
TEST(metadata_test, test_write_read) {
  Metadata metadata;
  metadata.btree_root_id = 2;
  metadata.data_ids.push_back(1);
  metadata.btree_ids.push_back(1);

  // test write
  std::stringstream ss;
  metadata.write(ss);
  std::cout << "wrote block size:" << ss.tellp() << std::endl;

  // test read from write
  Metadata read;
  read.read(ss);
  ASSERT_EQ(metadata, read);
}
