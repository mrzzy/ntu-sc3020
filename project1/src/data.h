#ifndef DATA_H
#define DATA_H 1;
/*
 * SC3020
 * Project 1
 * Data Block
 */
#include "block.h"
#include "id.h"
#include "record.h"
#include <cstdint>
#include <ctime>
#include <vector>

/** Data Block to store games.txt data */
class Data : public Block {
public:
  // no. of records the data block can store
  uint8_t capacity;
  /* Compute capacity based on filesystem block size */
  static uint8_t fs_capacity();

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
  // store home_team_wins boolean as uint8_t as vector<bool> is a bitmap
  std::vector<uint8_t> home_team_wins;

  // Block id to the next overflow data block if any or BLOCK_ID max value if
  // none
  BlockID next_id;

  Data() : capacity(fs_capacity()){};
  /** Insert the given record into the data block */
  RecordID insert(const Record &record);
  /** Get the record for given record ID */
  Record get(RecordID id);
  /** Current number of inserted records */
  uint8_t count() const { return record_pos.size(); }
  /** Minimum record key of any record stored the data block */
  Key key() const;
  /** Minimum record key of any record stored the data block */
  bool is_full() const { return count() >= capacity; }

  virtual void read(std::istream &in) override;
  virtual void write(std::ostream &out) const override;
  virtual BlockKind block_kind() const override { return BlockKindData; }
  bool operator==(const Data &other) const;
};

#endif /* ifndef DATA_H */
