/*
 * SC3020
 * Project 1
 * Memory Store
 */

#include "mem_store.h"
#include "block.h"
#include "id.h"
#include <memory>
#include <sstream>
#include <stdexcept>

BlockID MemStore::insert(std::shared_ptr<Block> block) {
  // record block id by block kind in metadata
  std::shared_ptr meta = get_meta();
  BlockID id = meta->new_id();
  meta->get_ids(block->block_kind()).push_back(id);
  set_meta(meta);

  // store block
  blocks.push_back(block);
  BlockKind kind = block->block_kind();

  return id;
}

void MemStore::update(BlockID block_id, std::shared_ptr<Block> block) {
  blocks[block_id] = block;
}

std::shared_ptr<Block> MemStore::get_block(BlockID id) const {
  // check if block id is valid
  if (id >= blocks.size()) {
    std::stringstream ss;
    ss << "MemStore::get: invalid block id" << id;
    throw std::runtime_error(ss.str());
  }
  return blocks[id];
}
