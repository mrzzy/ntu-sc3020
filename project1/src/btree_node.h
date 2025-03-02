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

  /** Construct empty BTree node. Only used for creating a empty node for reading. */
  BTreeNode();
  /** Construct Btree node with initial pointer */
  BTreeNode(BlockID pointer) : BTreeNode() { pointers.push_back(pointer); }

  /** Read the data block as bytes into the given stream */
  virtual void read(std::istream &in) override;
  /** Write the data block as bytes into the given stream */
  virtual void write(std::ostream &out) const override;
  /** 
   * Insert the given key & greater than or equal pointer 'ge_pointer'.
   * 'ge_pointer' points to block containing greater or equal keys.
   */
  void insert(Key key, BlockID ge_pointer);

  // Equality operator
  bool operator==(const BTreeNode &other) const;
  /** No. of keys currently maintained by the B+Tree node */
  uint16_t size() const { return keys.size(); }
};

#endif /* ifndef BTREE_H */
