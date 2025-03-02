/*
 * SC3020
 * Project 1
 * BTree Node Tests
 */

#include "btree_node.h"
#include <gtest/gtest.h>

TEST(btree_node_test, test_read_write_insert) {
  // add n_keys + 1 pointer
  BTreeNode node(0);
  for (uint16_t i = node.capacity; i > 0; i--) {
    node.insert(i, i);
  }
  
  // check keys inserted in sorted order
  ASSERT_LE(node.keys[1], node.keys[2]);
  ASSERT_LE(node.keys[node.capacity - 2], node.keys[node.capacity-1]);

  std::stringstream ss;
  node.write(ss);
  std::cout << "wrote block size:" << ss.tellp() << std::endl;

  BTreeNode read;
  read.read(ss);
  ASSERT_EQ(node, read);
}
