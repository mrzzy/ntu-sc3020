/*
 * SC3020
 * Project 1
 * BTree
 */

#include "btree.h"
#include "id.h"
#include <cmath>
#include <cstdint>
#include <map>
#include <memory>
#include <stdexcept>
#include <vector>

std::map<Key, BlockID>
BTree::load_leaf(const std::map<Key, BlockID> &key_pointers) {
  // create leaf nodes from keys in sorted order
  // by default std::map maintains keys in sorted order
  std::map<Key, BlockID> propagate;
  std::shared_ptr<BTreeNode> leaf;
  BlockID leaf_id = BLOCK_NULL;

  for (const auto &[key, pointer] : key_pointers) {
    if (leaf.get() == nullptr || leaf->is_full()) {
      // leaf node full / doesn't exist: create a new leaf node
      std::shared_ptr<BTreeNode> next =
          std::make_shared<BTreeNode>(BTreeNodeKindLeaf, capacity);
      BlockID next_id = store.insert(leaf);

      // track keys & pointers to propagate to parent node(s)
      propagate[key] = next_id;

      if (leaf_id != BLOCK_NULL) {
        // reassign pointer of prevous leaf to next leaf
        leaf->pointers[leaf->pointers.size() - 1] = next_id;
        store.update(leaf_id, leaf);
      }
      leaf = next;
      leaf_id = next_id;
    }

    // insert key pointer pair into btree ode
    leaf->insert(key, pointer);
    store.update(leaf_id, leaf);
  }
  return propagate;
}

std::map<Key, BlockID>
BTree::load_internal(const std::map<Key, BlockID> &key_pointers) {
  if (key_pointers.size() < 2) {
    throw std::runtime_error(
        "Internal B+Tree level at least 2 key pointer pairs to buik load.");
  }

  // create internal nodes from keys in sorted order
  // by default std::map maintains keys in sorted order
  std::map<Key, BlockID> propagate;
  std::shared_ptr<BTreeNode> internal;
  BlockID internal_id = BLOCK_NULL;

  // internal node perform (about) half splits
  // to simulate this when bulk loading insert only about half the keys
  // before "splitting" to inserting on a fresh internal node.
  const uint16_t split_at = std::ceil(capacity / 2.0);
  for (const auto &[key, pointer] : key_pointers) {
    if (internal.get() == nullptr || internal->size() >= split_at) {
      // internal node full / doesn't exist: create a new internal node
      internal = std::make_shared<BTreeNode>(BTreeNodeKindInternal, capacity);
      internal_id = store.insert(internal);

      // track keys & pointers to propagate to parent node(s)
      propagate[key] = internal_id;
    }
    // insert key pointer pair into btree node
    internal->insert(key, pointer);
    store.update(internal_id, internal);
  }

  return propagate;
}

int BTree::bulk_load(const std::map<Key, BlockID> &key_pointers) {
  int n_levels = 0;

  if (key_pointers.size() <= 0) {
    // nothing to do
    return n_levels;
  }

  // load leaf nodes
  std::map<Key, BlockID> propagate = load_leaf(key_pointers);
  n_levels++;
  if (propagate.size() == 1) {
    // leaf level is root node
    return n_levels;
  }

  // repeated propgate key-pointers pairs to internal node levels
  // until root node when only 1 key-pointer pair remains
  while (propagate.size() > 1) {
    propagate = load_internal(propagate);
    n_levels++;
  }

  // assign root node
  root = propagate.begin()->second;

  return n_levels;
}
