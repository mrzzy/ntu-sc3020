/*
 * SC3020
 * Project 1
 * BTree Node Tests
 */

#include "btree_node.h"
#include <gtest/gtest.h>
TEST(btree_node_test, test_read_write) {
  BTreeNode node;
  for (uint16_t i = 0; i < node.n_keys; i++) {
    node.keys.push_back(i);
    node.pointers.push_back(i);
  }
  // add n_keys + 1 pointer
  node.pointers.push_back(node.n_keys);

  std::stringstream ss;
  node.write(ss);
  std::cout << "wrote block size:" << ss.tellp() << std::endl;

  BTreeNode read;
  read.read(ss);
  ASSERT_EQ(node, read);
}
