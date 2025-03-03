/*
 * SC3020
 * Project 1
 * Memory Store
 */

#include "mem_store.h"
#include "id.h"
#include <memory>
#include <sstream>
#include <stdexcept>

BlockID MemStore::insert(std::shared_ptr<Block> block) {
  BlockID id = blocks.size();
  blocks.push_back(block);
  return id;
}

void MemStore::update(BlockID block_id, std::shared_ptr<Block> block) {
  blocks[block_id] = block;
}

std::shared_ptr<Block> MemStore::get(BlockID id) const {
  // check if block id is valid
  if (id >= blocks.size()) {
    std::stringstream ss;
    ss << "MemStore::get: invalid block id" << id;
    throw std::runtime_error(ss.str());
  }
  return blocks[id];
}
