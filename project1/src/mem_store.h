/*
 * SC3020
 * Project 1
 * Memory Store
 */

#include "block.h"
#include <memory>
#include <unordered_map>
#include <vector>
#ifndef MEM_STORE_H
#define MEM_STORE_H 1

#include "store.h"

/** In-memory block storage */
class MemStore : public Store {
public:
  // Blocks stored by memory store
  std::vector<std::shared_ptr<Block>> blocks;
  std::unordered_map<BlockKind, std::vector<BlockID>> block_kind_ids;

  virtual BlockID insert(std::shared_ptr<Block> block) override;
  virtual void update(BlockID block_id, std::shared_ptr<Block> block) override;
  virtual std::shared_ptr<Block> get_block(BlockID id) const override;
  virtual std::vector<BlockID> kind_ids(BlockKind kind) const override;
};
#endif /* ifndef MEM_STORE_H */
