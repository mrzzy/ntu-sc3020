#include "store.h"
#ifndef BTREE_H
#define BTREE_H 1
/*
 * SC3020
 * Project 1
 * BTree
 */

#include "btree_node.h"
#include "id.h"
#include <map>

class BTree {
  // TODO(mrzzy): add generic method for block id -> btree node lookup.
public:
  // Block store used to provide storage for btree nodes
  Store &store;

  // block id of the root BTree index block or BlockID max value if no root
  // block exists, ie. btree is empty.
  BlockID root;

  /* Construct an empty btree backed by the given block store */
  BTree(Store &store) : store(store), root(BLOCK_NULL){};
  /** Bulk load the given keys-block pointer mapping into a B+Tree */
  void bulk_load(const std::map<Key, BlockID> &key_pointers);

  /**
   * Bulk load the given key-pointer pairs into leaf BTreeNodes.
   * Returns key-pointers pairs to propgate to parent internal nodes.
   */
  std::map<Key, BlockID> load_leaf(const std::map<Key, BlockID> &key_pointers);
};

#endif /* ifndef BTREE_H */
