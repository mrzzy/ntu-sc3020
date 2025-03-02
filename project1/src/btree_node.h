#include <cstdint>
#ifndef BTREE_H
#define BTREE_H 1
/*
 * SC3020
 * Project 1
 * BTree Node
 */

#include "data.h"
#include "id.h"
#include <vector>

#include "block.h"

/** B+Tree Index node block */
class BTreeNode : Block {
public:
  /** No. of keys maintained by the B+Tree node */
  uint16_t n_keys;
  /** Keys maintained by the B+Tree node */
  std::vector<Key> keys;
  /** Block Pointer(s) (BlockIDs) maintained by the BTree node. */
  std::vector<BlockID> pointers;

  BTreeNode();

  /** Read the data block as bytes into the given stream */
  virtual void read(std::istream &in) override;
  /** Write the data block as bytes into the given stream */
  virtual void write(std::ostream &out) const override;

  // Equality operator
  bool operator==(const BTreeNode &other) const;
};

#endif /* ifndef BTREE_H */
