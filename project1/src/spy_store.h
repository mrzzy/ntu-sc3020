#include <cstdint>
#ifndef SPY_STORE_H
#define SPY_STORE_H 1
/*
 * SC3020
 * Project 1
 * Spy Store
 */
#include "block.h"
#include "store.h"
#include <unordered_map>

enum SpyOp {
  SpyOpRead = 0,
  SpyOpWrite = 1,
};

/** SpyStore wraps another store to intercept calls and collect statistics */
class SpyStore : public Store {
public:
  std::unique_ptr<Store> store;
  std::unordered_map<SpyOp, std::unordered_map<BlockKind, uint32_t>> counts;

  SpyStore(std::unique_ptr<Store> store) : store(std::move(store)){};

  virtual BlockID insert(std::shared_ptr<Block> block) override;
  virtual void update(BlockID block_id, std::shared_ptr<Block> block) override;
  virtual std::shared_ptr<Block> get_block(BlockID id) override;
  virtual std::shared_ptr<Metadata> get_meta() const override;
  virtual void set_meta(std::shared_ptr<Metadata> metadata) override;
};

#endif /* ifndef SPY_STORE_H */
