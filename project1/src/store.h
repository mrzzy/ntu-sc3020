#ifndef STORE_H
#define STORE_H 1
/*
 * SC3020
 * Project 1
 * Store
 */

#include "block.h"
#include "id.h"
#include <memory>
/** Abstract Block Store responsible for storing / fetching Blocks */
class Store {
public:
  /** Inserts the given block into storage */
  virtual BlockID insert(std::shared_ptr<Block> block) = 0;
  /** Update the block with given block_id in storage */
  virtual void update(BlockID block_id, std::shared_ptr<Block> block) = 0;
  /** Gets the given block for the given block id */
  virtual std::shared_ptr<Block> get(BlockID id) const = 0;
};
#endif /* ifndef STORE_H */
