#ifndef ID_H
#define ID_H 1
/*
 * SC3020
 * Project 1
 * Record
 */

#include <cstdint>

using BlockID = uint16_t;
using RecordID = uint16_t;
using Key = uint16_t;

/** Identifier that uniquely identifies a data record */
struct ID {
public:
  /** ID of the data block that stores the record */
  BlockID block_id;
  /** ID of the record qualified by data block */ 
  RecordID record_id;
};
#endif /* ifndef ID_H */
