/*
 * SC3020
 * Project 1
 * Metadata Block
 */

#include "metadata.h"
#include "block.h"
#include "data.h"
#include "fs.h"
#include <algorithm>
#include <cstdint>
#include <iterator>
#include <ostream>
#include <sstream>
#include <stdexcept>
#include <utility>
#include <vector>

void Metadata::read(std::istream &in) {
  // read header:
  // - 1 uint16_t storing btree root id
  in.read(reinterpret_cast<char *>(&btree_root_id), sizeof(btree_root_id));
  // - 1 uint16_t storing no. of data block ids
  uint16_t data_size;
  in.read(reinterpret_cast<char *>(&data_size), sizeof(data_size));
  // - 1 uint16_t storing no. of btree block ids
  uint16_t btree_size;
  in.read(reinterpret_cast<char *>(&btree_size), sizeof(btree_size));
  // read body:
  // - data block ids
  read_vec(in, data_ids, data_size);
  // - btree block ids
  read_vec(in, btree_ids, btree_size);
}

void Metadata::write(std::ostream &out) const {
  // write header:
  // - 1 uint16_t storing btree root id
  out.write(reinterpret_cast<const char *>(&btree_root_id),
            sizeof(btree_root_id));
  // - 1 uint16_t storing no. of data block ids
  uint16_t data_size = data_ids.size();
  out.write(reinterpret_cast<const char *>(&data_size), sizeof(data_size));
  // - 1 uint16_t storing no. of btree block ids
  uint16_t btree_size = btree_ids.size();
  out.write(reinterpret_cast<const char *>(&btree_size), sizeof(btree_size));
  // write body:
  // - data block ids
  write_vec(data_ids, out);
  // - btree block ids
  write_vec(btree_ids, out);
}

std::vector<BlockID> &Metadata::get_ids(BlockKind kind) {
  if (kind == BlockKindData) {
    return data_ids;
  } else if (kind == BlockKindBTreeNode) {
    return btree_ids;
  } else {
    throw std::runtime_error("Metadata::get_ids: Unsupported block type.");
  }
}

/**
 * Perform binary search of the given block id on the given block ids.
 * Returns the index of the block id if found, length of the vector otherwise.
 **/
size_t find_block_id(std::vector<BlockID> ids, BlockID id) {
  auto it = std::lower_bound(ids.begin(), ids.end(), id);
  if (it == ids.end() || *it != id) {
    // not found
    return ids.size();
  }
  return std::distance(ids.begin(), it);
}

std::pair<BlockKind, size_t> Metadata::lookup(BlockID id) const {
  // lookup id in data block ids
  size_t index = find_block_id(data_ids, id);
  if (index < data_ids.size()) {
    // found in data_ids
    return std::make_pair(BlockKindData, index);
  }
  // lookup id in btree block ids
  index = find_block_id(btree_ids, id);
  if (index < btree_ids.size()) {
    // found in btree_ids
    return std::make_pair(BlockKindBTreeNode, index);
  }

  std::stringstream ss;
  ss << "Metadata::lookup: unknown block id: " << id;
  throw std::runtime_error(ss.str());
}

BlockID Metadata::new_id() const {
  return data_ids.size() + btree_ids.size();
}

bool Metadata::operator==(const Metadata &other) const {
  return (btree_root_id == other.btree_root_id) &&
         (data_ids == other.data_ids) && (btree_ids == other.btree_ids);
}
