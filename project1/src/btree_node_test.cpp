/*
 * SC3020
 * Project 1
 * BTree Node Tests
 */

#include "btree_node.h"
#include <gtest/gtest.h>

TEST(btree_node_test, test_read_write_leaf) {
  BTreeNode node(BTreeNodeKindLeaf);
  for (uint16_t i = node.capacity; i > 0; i--) {
    node.insert(i, i);
  }
  std::stringstream ss;
  node.write(ss);
  std::cout << "wrote block size:" << ss.tellp() << std::endl;

  BTreeNode read;
  read.read(ss);
  ASSERT_EQ(node.kind, BTreeNodeKindLeaf);
  ASSERT_EQ(node, read);
}

TEST(btree_node_test, test_read_write_internal) {
  BTreeNode node(BTreeNodeKindInternal);
  for (uint16_t i = node.capacity; i > 0; i--) {
    node.insert(i, i);
  }
  std::stringstream ss;
  node.write(ss);
  std::cout << "wrote block size:" << ss.tellp() << std::endl;

  BTreeNode read;
  read.read(ss);
  ASSERT_EQ(node.kind, BTreeNodeKindInternal);
  ASSERT_EQ(node, read);
}

TEST(btree_node_test, insert_leaf) {
  BTreeNode node(BTreeNodeKindLeaf);
  for (uint16_t i = node.capacity; i > 0; i--) {
    node.insert(i, i);
  }

  // check keys inserted in sorted order
  for (uint16_t i = 0; i < node.size() - 1; i++) {
    EXPECT_LE(node.keys[i], node.keys[i + 1]);
  }
}

TEST(btree_node_test, insert_internal) {
  BTreeNode node(BTreeNodeKindInternal);

  // check first insertion does not introduce key
  node.insert(555, 555);
  ASSERT_EQ(node.keys.size(), 0);
  ASSERT_EQ(node.pointers.size(), 1);

  for (uint16_t i = node.capacity - 1; i > 0; i--) {
    node.insert(i, i);
  }

  // check keys inserted in sorted order
  for (uint16_t i = 0; i < node.size(); i++) {
    ASSERT_LE(node.keys[i], node.keys[i + 1]);
  }
}
