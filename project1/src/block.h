#ifndef BLOCK_H
#define BLOCK_H 1
/*
 * SC3020
 * Project 1
 * Block
 */

#include <istream>
#include <ostream>
/** Abstract Block that can be written to and read from bytes */
class Block {
  /** Read the data block as bytes into the given stream */
  virtual void read(std::istream &in) = 0;
  /** Write the data block as bytes into the given stream */
  virtual void write(std::ostream &out) const = 0;
};

#endif /* ifndef BLOCK_H */
