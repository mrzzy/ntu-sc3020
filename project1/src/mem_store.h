/*
 * SC3020
 * Project 1
 * Memory Store
 */

#include "block.h"
#include <memory>
#include <vector>
#ifndef MEM_STORE_H
#define MEM_STORE_H 1

#include "store.h"

/** In-memory block storage */
class MemStore : public Store {
public:
  // Blocks stored by memory store
  std::vector<std::shared_ptr<Block>> blocks;

  /** Inserts the given block into storage with the given block id. */
  virtual BlockID insert(std::shared_ptr<Block> block) override;
  /** Gets the given block for the given block id */
  virtual std::shared_ptr<Block> get(BlockID id) const override;
};
#endif /* ifndef MEM_STORE_H */
