/*
 * SC3020
 * Project 1
 * BTree Node Tests
 */

#include "btree_node.h"
#include <gtest/gtest.h>
TEST(btree_node_test, test_read_write) {
  // add n_keys + 1 pointer
  BTreeNode node(0);
  for (uint16_t i = 0; i < node.capacity - 2; i++) {
    node.keys.push_back(i);
    node.pointers.push_back(i);
  }

  std::stringstream ss;
  node.write(ss);
  std::cout << "wrote block size:" << ss.tellp() << std::endl;

  BTreeNode read;
  read.read(ss);
  ASSERT_EQ(node, read);
}
