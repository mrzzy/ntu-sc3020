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
  /** Data block IDs in order of storage */
  std::vector<BlockID> data_ids;
  /** Index block ids in order of storage */
  std::vector<BlockID> btree_ids;

  Metadata() : btree_root_id(BLOCK_NULL){};

  virtual void read(std::istream &in) override;
  virtual void write(std::ostream &out) const override;
  virtual BlockKind block_kind() const override { return BlockKindMetadata; }
  std::vector<BlockID> &get_ids(BlockKind kind);
  bool operator==(const Metadata &other) const;
};

#endif /* ifndef META_H */
