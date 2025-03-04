#ifndef BLOCK_H
#define BLOCK_H 1
/*
 * SC3020
 * Project 1
 * Block
 */

#include <istream>
#include <ostream>

/** Block kind determines the variant of block. */
enum BlockKind {
  BlockKindData = 0,
  BlockKindIndex = 1,
};

/** Abstract Block that can be written to and read from bytes */
class Block {
  /** Read the block as bytes into the given stream */
  virtual void read(std::istream &in) = 0;
  /** Write the block as bytes into the given stream */
  virtual void write(std::ostream &out) const = 0;
  /** Get the block kind of this block */
  virtual BlockKind block_kind() const = 0;
};
#endif /* ifndef BLOCK_H */
