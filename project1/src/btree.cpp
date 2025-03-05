/*
 * SC3020
 * Project 1
 * BTree
 */

#include "btree.h"
#include "database.h"
#include "id.h"
#include <cmath>
#include <cstdint>
#include <limits>
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
      BlockID next_id = store.insert(next);

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

size_t BTree::bulk_load(const std::map<Key, BlockID> &key_pointers) {
  size_t n_levels = 0;

  if (key_pointers.size() <= 0) {
    // nothing to do
    return n_levels;
  }

  // load leaf nodes
  std::map<Key, BlockID> propagate = load_leaf(key_pointers);
  n_levels++;
  if (propagate.size() == 1) {
    // leaf level is root node
    set_root(propagate.begin()->second);
    return n_levels;
  }

  // repeated propgate key-pointers pairs to internal node levels
  // until root node when only 1 key-pointer pair remains
  while (propagate.size() > 1) {
    propagate = load_internal(propagate);
    n_levels++;
  }

  // assign root node
  set_root(propagate.begin()->second);

  return n_levels;
}

std::shared_ptr<BTreeNode> BTree::lookup(Key key) const {
  if (root() == BLOCK_NULL) {
    // empty btree: nothing to do
    return nullptr;
  }

  // traverse >= 0 internal nodes to get to leaf nodes
  std::shared_ptr node = store.get<BTreeNode>(root());
  while (node->kind != BTreeNodeKindLeaf) {
    // find first key in node >= search key
    auto key_it = std::lower_bound(node->keys.begin(), node->keys.end(), key);
    auto index = std::distance(node->keys.begin(), key_it);

    BlockID next_id;
    if (key_it == node->keys.end()) {
      // follow last block pointer
      next_id = node->pointers[node->pointers.size() - 1];
    } else if (key >= *key_it) {
      // follow block pointer to right of key
      next_id = node->pointers[index + 1];
    } else {
      // follow block pointer to left of key
      next_id = node->pointers[index];
    }
    node = store.get<BTreeNode>(next_id);
  }

  // reached leaf node with key
  return node;
}

BlockID BTree::get(Key key) const {
  std::shared_ptr node = lookup(key);
  if (!node) {
    // btree node not found
    return BLOCK_NULL;
  }
  // find first key in node == search key
  auto key_it = std::lower_bound(node->keys.begin(), node->keys.end(), key);
  if (key_it != node->keys.end() && key == *key_it) {
    // found key
    auto index = std::distance(node->keys.begin(), key_it);
    return node->pointers[index];
  }
  // key not found
  return BLOCK_NULL;
}

std::vector<BlockID> BTree::range(Key begin, Key end) const {
  if (is_empty()) {
    // b tree is empty: nothing to do
    return {};
  }

  // lookup begin key in btree
  std::shared_ptr node = lookup(begin);
  if (!node) {
    // fallback to start of btree
    node = lookup(std::numeric_limits<Key>::min());
  }

  // find first key in node >= begin key
  auto key_it = std::lower_bound(node->keys.begin(), node->keys.end(), begin);

  // collect block pointers until see a key greater than end key
  std::vector<BlockID> pointers;
  while (key_it != node->keys.end() && *key_it <= end) {
    auto index = std::distance(node->keys.begin(), key_it);
    pointers.push_back(node->pointers[index]);

    // advance to next key
    key_it++;
    if (key_it == node->keys.end()) {
      // advance to next btree node as current node is exhausted
      if (node->pointers.size() <= 0) {
        throw std::runtime_error(
            "BTreeNode::range: invalid btree node with key(s) but no pointers");
      }
      BlockID next_id = node->pointers.back();
      if (next_id == BLOCK_NULL) {
        // exhausted all btree nodes we are done
        return pointers;
      }
      node = store.get<BTreeNode>(next_id);
      key_it = node->keys.begin();
    }
  }

  return pointers;
}
