#include <ostream>
#ifndef BLOCK_H
#define BLOCK_H 1;
/*
 * SC3020
 * Project 1
 * Data Block
 */
#include "id.h"
#include "record.h"
#include <cstdint>
#include <ctime>
#include <vector>

/** Data Block to store games.txt data */
class Data {
public:
  // no. of records the data block can store
  uint8_t capacity;
  
  // maps record id to position of record in block
  std::vector<RecordID> record_pos;
  // fields stored in a columar format (structure of arrays) for better data
  // locality when scanning by single field.
  // fields stored clustered (ordered) by key
  std::vector<time_t> game_date_est;
  std::vector<uint32_t> team_id_home;
  std::vector<float> fg_pct_home;
  std::vector<float> ft_pct_home;
  std::vector<float> fg3_pct_home;
  std::vector<uint8_t> pts_home;
  std::vector<uint8_t> ast_home;
  std::vector<uint8_t> reb_home;
  std::vector<uint8_t> home_team_wins;

  Data();

  /** Insert the given record into the data block */
  RecordID insert(const Record &record);
  /** Get the record for given record ID */
  Record get(RecordID id);
  /** Find the record Ids with the given key */
  std::vector<RecordID> find(Key key);
  /** Current number of inserted records */
  int size() const { return record_pos.size(); }
  /** Write the data block as bytes into the given stream */
  void write(std::ostream &out) const;
};

#endif /* ifndef BLOCK_H */
