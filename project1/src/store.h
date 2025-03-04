#ifndef STORE_H
#define STORE_H 1
/*
 * SC3020
 * Project 1
 * Store
 */

#include "block.h"
#include "id.h"
#include "metadata.h"
#include <memory>

/**
 * Abstract Block Store responsible for storing / fetching Blocks
 * All stores must create and maintain the lifecycle of their own Metadata
 * block.
 */
class Store {
public:
  /** Inserts the given block into storage */
  virtual BlockID insert(std::shared_ptr<Block> block) = 0;
  /**
   * Update the block with given block_id in storage.
   * Changing of block kind using update is not permitted.
   */
  virtual void update(BlockID block_id, std::shared_ptr<Block> block) = 0;
  /** Gets the given block for the given block id */
  virtual std::shared_ptr<Block> get_block(BlockID id) = 0;
  /** Gets the Metadata block of this store */
  virtual std::shared_ptr<Metadata> get_meta() const = 0;
  /** Updates the Metadata block of this store */
  virtual void set_meta(std::shared_ptr<Metadata> metadata) = 0;
  /** Persist changes made to blocks in this store to storage */
  virtual void persist() = 0;
  /** Get block with given block id with casting to derived block type */
  template <typename T> std::shared_ptr<T> get(BlockID id) {
    return std::dynamic_pointer_cast<T>(get_block(id));
  }

  /** Register BlockID for the given new block in metadata */
  BlockID register_id(std::shared_ptr<Block> block);
};
#endif /* ifndef STORE_H */
