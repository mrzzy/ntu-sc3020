/*
 * SC3020
 * Project 1
 * Metadata Block
 */

#include "metadata.h"
#include "fs.h"
#include <cstdint>

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

bool Metadata::operator==(const Metadata &other) const {
  return (btree_root_id == other.btree_root_id) &&
         (data_ids == other.data_ids) && (btree_ids == other.btree_ids);
}
