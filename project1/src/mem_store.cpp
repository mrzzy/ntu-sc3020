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
  // store block
  BlockID id = blocks.size();
  blocks.push_back(block);

  // record block id by block kind
  block_kind_ids[block->block_kind()].push_back(id);
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

std::vector<BlockID> MemStore::kind_ids(BlockKind kind) const {
  return block_kind_ids.at(kind);
}
