/*
 * SC3020
 * Project 1
 * BTree Node
 */

#include "btree_node.h"
#include "fs.h"
#include <cstddef>

BTreeNode::BTreeNode() {
  // determine btree node key capacity based on fs block size
  // header: 1 uint16_t storing number of keys in the in block
  size_t header_size = sizeof(uint16_t);
  size_t pointer_size = sizeof(BlockID);
  // ensure space for n keys and n+1 pointers + header
  capacity = (block_size() - header_size - pointer_size) /
           (sizeof(Key) + pointer_size);
}

void BTreeNode::read(std::istream &in) {
  // header: read n no. of keys
  uint16_t size_;
  in.read(reinterpret_cast<char *>(&size_), sizeof(size_));
  // read n keys
  read_vec(in, keys, size_);
  // read n+1 pointers
  read_vec(in, pointers, size_ + 1);
}

void BTreeNode::write(std::ostream &out) const {
  // header: no. of keys stored by btree block
  uint16_t size_ = size();
  out.write(reinterpret_cast<const char *>(&size_), sizeof(size_));
  // write btree keys & pointers
  write_vec(keys, out);
  write_vec(pointers, out);
}

// Directly implemented equality operator
bool BTreeNode::operator==(const BTreeNode &other) const {
  return capacity == other.capacity && keys == other.keys &&
         pointers == other.pointers;
}
