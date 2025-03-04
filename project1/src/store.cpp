/*
 * SC3020
 * Project 1
 * Store
 */

#include "store.h"

BlockID Store::register_id(std::shared_ptr<Block> block) {
  // record block id by block kind in metadata
  std::shared_ptr meta = get_meta();
  BlockID id = meta->new_id();
  meta->get_ids(block->block_kind()).push_back(id);
  set_meta(meta);
  return id;
}
