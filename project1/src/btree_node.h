#include <cstdint>
#ifndef BTREE_NODE_H
#define BTREE_NODE_H 1
/*
 * SC3020
 * Project 1
 * BTree Node
 */

#include "id.h"
#include <vector>

#include "block.h"

/** B+Tree Index Node kind */
enum BTreeNodeKind {
  BTreeNodeKindInternal = 0,
  BTreeNodeKindLeaf = 1,
};

/** B+Tree Index node block */
class BTreeNode : public Block {
public:
  /** No. of keys that can be maintained by the B+Tree node */
  uint16_t capacity;
  /** Keys maintained by the B+Tree node in aseconding sorted order. */
  std::vector<Key> keys;
  /** Block Pointer(s) (BlockIDs) maintained by the BTree node. */
  std::vector<BlockID> pointers;
  /** Type of node */
  BTreeNodeKind kind;

  /** Compute B+Tree node key capacity based on filesystem file size */
  static uint16_t fs_capacity();

  /** Construct empty BTree node */
  BTreeNode() : BTreeNode(BTreeNodeKindInternal){};
  /** Construct empty BTree node with the given kind */
  BTreeNode(BTreeNodeKind kind) : BTreeNode(kind, BTreeNode::fs_capacity()){};
  /** Construct empty BTree node with the given kind */
  BTreeNode(BTreeNodeKind kind, uint16_t capacity)
      : kind(kind), capacity(capacity){};

  /** Read the data block as bytes into the given stream */
  virtual void read(std::istream &in) override;
  /** Write the data block as bytes into the given stream */
  virtual void write(std::ostream &out) const override;
  /**
   * Insert the given key & greater than or equal pointer 'ge_pointer'.
   * 'ge_pointer' points to block containing greater or equal keys.
   */
  void insert(Key key, BlockID pointer);
  /** No. of keys currently maintained by the B+Tree node */
  uint16_t size() const { return keys.size(); }
  /** Whether the Btree node is currently full */
  bool is_full() const { return size() >= capacity; }

  // Equality operator
  bool operator==(const BTreeNode &other) const;
};

#endif /* ifndef BTREE_NODE_H */
