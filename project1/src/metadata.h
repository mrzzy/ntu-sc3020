#include <cstddef>
#include <utility>
#ifndef META_H
#define META_H 1
/*
 * SC3020
 * Project 1
 * Metadata Block
 */

#include "block.h"
#include "data.h"
#include <vector>

/** Metadata block to store storage metdata */
class Metadata : public Block {
public:
  /** ID of the B+Tree index root or BLOCK_NULL if non exists */
  BlockID btree_root_id;
  /** Data block IDs in order of storage and ascending block id  */
  std::vector<BlockID> data_ids;
  /** Index block ids in order of storage and asecnding block id */
  std::vector<BlockID> btree_ids;

  Metadata() : btree_root_id(BLOCK_NULL){};
  /** Get block ids for the given block type */
  std::vector<BlockID> &get_ids(BlockKind kind);
  /** Lookup the kind and storage position of the block with the given id */
  std::pair<BlockKind, size_t> lookup(BlockID id) const;
  /** Get a fresh unallocated block id */
  BlockID new_id() const;

  virtual void read(std::istream &in) override;
  virtual void write(std::ostream &out) const override;
  virtual BlockKind block_kind() const override { return BlockKindMetadata; }
  bool operator==(const Metadata &other) const;
};

#endif /* ifndef META_H */
