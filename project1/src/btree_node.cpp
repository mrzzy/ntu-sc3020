/*
 * SC3020
 * Project 1
 * BTree Node
 */

#include "btree_node.h"
#include "fs.h"
#include "id.h"
#include <algorithm>
#include <bitset>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <ostream>

BTreeNode::BTreeNode() : kind(BTreeNodeKindInternal) {
  // determine btree node key capacity based on fs block size
  // header 1 uint16_t:
  // * msb (15) bit: node kind
  // * 14-0 bit: storing number of keys in the in block
  size_t header_size = sizeof(uint16_t);
  size_t pointer_size = sizeof(BlockID);
  // ensure space for n keys and n+1 pointers + header
  capacity = (block_size() - header_size - pointer_size) /
             (sizeof(Key) + pointer_size);
}

BTreeNode::BTreeNode(BlockID pointer, BTreeNodeKind kind) : BTreeNode() {
  pointers.push_back(pointer);
  this->kind = kind;
}

/** mask used to obtain btree node kind bit from header */
constexpr uint16_t HEADER_KIND_BIT = 15;

void BTreeNode::read(std::istream &in) {
  // header: read n no. of keys & node type
  uint16_t size_kind;
  in.read(reinterpret_cast<char *>(&size_kind), sizeof(size_kind));
  kind = static_cast<BTreeNodeKind>((size_kind & (1 << HEADER_KIND_BIT)) >>
                                    HEADER_KIND_BIT);
  uint16_t size_ = size_kind & (~(1 << HEADER_KIND_BIT));

  // read n keys
  read_vec(in, keys, size_);
  // read n+1 pointers
  read_vec(in, pointers, size_ + 1);
}

void BTreeNode::write(std::ostream &out) const {
  // header: no. of keys stored by btree block & node type
  uint16_t size_kind = size() & (~(1 << HEADER_KIND_BIT));
  size_kind |= (static_cast<uint16_t>(kind) << HEADER_KIND_BIT);
  out.write(reinterpret_cast<const char *>(&size_kind), sizeof(size_kind));

  // write btree keys & pointers
  write_vec(keys, out);
  write_vec(pointers, out);
}

void BTreeNode::insert(Key key, BlockID ge_pointer) {
  // reject inserts exceeding capacity
  if (size() >= capacity) {
    throw std::runtime_error(
        "BTreeNode::insert(): insert exceeds block capacity");
  }

  // locate insertion position: before next greater key
  auto insert_it = std::upper_bound(keys.begin(), keys.end(), key);
  auto insert_at = std::distance(keys.begin(), insert_it);

  // insert key
  keys.insert(insert_it, key);
  // insert ge_pointer at key + 1 position since there is n_keys + 1 pointers
  pointers.insert(pointers.begin() + insert_at + 1, ge_pointer);
}

bool BTreeNode::operator==(const BTreeNode &other) const {
  return capacity == other.capacity && keys == other.keys &&
         pointers == other.pointers && kind == other.kind;
}
