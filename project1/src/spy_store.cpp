/*
 * SC3020
 * Project 1
 * Spy Store
 */

#include "spy_store.h"

BlockID SpyStore::insert(std::shared_ptr<Block> block) {
  counts[SpyOpWrite][block->block_kind()]++;
  return store->insert(std::move(block));
}

void SpyStore::update(BlockID block_id, std::shared_ptr<Block> block) {
  counts[SpyOpWrite][block->block_kind()]++;
  store->update(block_id, std::move(block));
}

std::shared_ptr<Block> SpyStore::get_block(BlockID id) {
  counts[SpyOpRead][get_meta()->lookup(id)]++;
  return store->get_block(id);
}

std::shared_ptr<Metadata> SpyStore::get_meta() const {
  return store->get_meta();
}

void SpyStore::set_meta(std::shared_ptr<Metadata> metadata) {
  store->set_meta(metadata);
}
