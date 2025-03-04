/*
 * SC3020
 * Project 1
 * Memory Store
 */

#include "block.h"
#include "metadata.h"
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
  std::shared_ptr<Metadata> meta;

  MemStore() : meta(std::make_shared<Metadata>()){};

  virtual BlockID insert(std::shared_ptr<Block> block) override;
  virtual void update(BlockID block_id, std::shared_ptr<Block> block) override;
  virtual std::shared_ptr<Block> get_block(BlockID id) const override;
  virtual std::vector<BlockID> kind_ids(BlockKind kind) const override;
  virtual std::shared_ptr<Metadata> get_meta() const override { return meta; }
  virtual void set_meta(std::shared_ptr<Metadata> metadata) override {
    meta = metadata;
  }
};
#endif /* ifndef MEM_STORE_H */
