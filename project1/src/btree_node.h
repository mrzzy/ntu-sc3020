#include <cstddef>
#include <cstdint>
#ifndef BTREE_H
#define BTREE_H 1
/*
 * SC3020
 * Project 1
 * BTree Node
 */

#include "id.h"
#include <vector>

#include "block.h"

/** B+Tree Index node block */
class BTreeNode : Block {
public:
  /** No. of keys that can be maintained by the B+Tree node */
  uint16_t capacity;
  /** Keys maintained by the B+Tree node in aseconding sorted order. */
  std::vector<Key> keys;
  /** Block Pointer(s) (BlockIDs) maintained by the BTree node. */
  std::vector<BlockID> pointers;

  BTreeNode();

  /** Read the data block as bytes into the given stream */
  virtual void read(std::istream &in) override;
  /** Write the data block as bytes into the given stream */
  virtual void write(std::ostream &out) const override;
  /** Insert the given key into the btree node */
  void insert(Key key);
  // Equality operator
  bool operator==(const BTreeNode &other) const;
  /** No. of keys currently maintained by the B+Tree node */
  size_t size() const { return keys.size(); }
};

#endif /* ifndef BTREE_H */
