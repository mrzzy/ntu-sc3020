#ifndef BTREE_H
#define BTREE_H 1
/*
 * SC3020
 * Project 1
 * BTree
 */
#include "btree_node.h"
#include "id.h"
#include "store.h"
#include <cstdint>
#include <map>
#include <vector>

class BTree {
public:
  // Key capacity of B+Tree nodes used to make up B+tree
  uint16_t capacity;

  // Block store used to provide storage for btree nodes
  Store &store;

  /* Construct an empty btree backed by the given block store */
  BTree(Store &store) : BTree(store, BTreeNode::fs_capacity()) {}
  /* Construct an empty btree backed by the given block store using nodes of
   * given capacity. */
  BTree(Store &store, uint16_t capacity) : capacity(capacity), store(store){};

  /**
   * Bulk load the given key-pointer pairs into leaf BTreeNodes.
   * Returns:
   * - >=2: key-pointers pairs to propagate to parent internal nodes.
   * - 1 key pointer pair: containing block id of root node.
   */
  std::map<Key, BlockID> load_leaf(const std::map<Key, BlockID> &key_pointers);

  /**
   * Bulk load the given key-pointer pairs into internal BTreeNodes.
   * Returns:
   *  - >=2 key-pointers pairs to propagate to parent internal nodes.
   *  - 1 key pointer pair: containing block id of root node.
   */
  std::map<Key, BlockID>
  load_internal(const std::map<Key, BlockID> &key_pointers);

  /**
   * Bulk load the given keys-block pointer mapping into a B+Tree.
   * Returns the number of levels in resulting B+tree
   */
  size_t bulk_load(const std::map<Key, BlockID> &key_pointers);

  /** Look the B+Tree node that MAY store the given key in the B+Tree */
  std::shared_ptr<BTreeNode> lookup(Key key) const;

  /**
   * Lookup block pointer with the given key in the B+Tree.
   * Returns BLOCK_NULL if no such block pointer is found.
   */
  BlockID get(Key key) const;

  /**
   * Lookup block pointers starting given 'begin' key and ending in 'end' key
   * (inclusive). Returns empty vector if no block pointers mathch
   */
  std::vector<BlockID> range(Key begin, Key end) const;

  /*
   * block id of the root BTree index block or BlockID max value if no root
   * block exists, ie. btree is empty.
   */
  BlockID root() const { return store.get_meta()->btree_root_id; }

  /** Set the block id of root B+Tree index block */
  void set_root(BlockID id) {
    std::shared_ptr<Metadata> meta = store.get_meta();
    meta->btree_root_id = id;
    store.set_meta(meta);
  }

  /** Whether the B+Tree is currently empty */
  bool is_empty() const {
    return store.get_meta()->btree_root_id == BLOCK_NULL;
  }
};

#endif /* ifndef BTREE_H */
