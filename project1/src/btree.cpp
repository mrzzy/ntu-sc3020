/*
 * SC3020
 * Project 1
 * BTree
 */

#include "btree.h"
#include "id.h"
#include <map>
#include <memory>
#include <vector>

// void BTree::bulk_load(const std::map<Key, BlockID> &keys) {}

std::map<Key, BlockID>
BTree::load_leaf(const std::map<Key, BlockID> &key_pointers) {
  // create leaf nodes from keys in sorted order
  // by default std::map mains has keys in sorted order
  std::map<Key, BlockID> propagate;
  std::shared_ptr<BTreeNode> leaf;

  BlockID block_id = BLOCK_NULL;
  for (const auto &[key, pointer] : key_pointers) {
    if (leaf.get() == nullptr || leaf->is_full()) {
      std::shared_ptr<BTreeNode> next =
          std::make_shared<BTreeNode>(BTreeNodeKindLeaf);
      BlockID next_id = store.insert(leaf);

      // track keys & pointers to propagate to internal node
      propagate[key] = next_id;

      if (block_id != BLOCK_NULL) {
        // reassign pointer of prevous leaf to next leaf
        leaf->pointers[leaf->pointers.size() - 1] = next_id;
        store.update(block_id, leaf);
      }
      leaf = next;
      block_id = next_id;
    }

    // insert key pointer pair into btree ode
    leaf->insert(key, pointer);
    store.update(block_id, leaf);
  }
  return propagate;
}
