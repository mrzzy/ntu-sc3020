/*
 * SC3020
 * Project 1
 * Data Block
 */

#include "data.h"
#include "fs.h"
#include "id.h"
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <iostream>
#include <iterator>
#include <limits>
#include <ostream>
#include <sstream>
#include <stdexcept>
#include <vector>

#define KEY_COL fg_pct_home

uint8_t Data::fs_capacity() {
  // determine data block record capacity from fs block size
  // header: 
  // - 1 uint8_t storing number of records in block
  // - 1 block id (2 bytes) pointer to next block (if any)
  size_t header_size = sizeof(uint8_t) + sizeof(next_id);
  return (block_size() - header_size) / Record::size();
}

RecordID Data::insert(const Record &record) {
  // reject inserts exceeding capacity
  if (is_full()) {
    throw std::runtime_error("Data::insert(): insert exceeds block capacity");
  }

  // locate insertion position: before next greater key
  auto insert_it = std::upper_bound(
      KEY_COL.begin(), KEY_COL.end(), record.key(),
      [](Key key, float value) { return key < Record::to_key(value); });
  auto insert_at = std::distance(KEY_COL.begin(), insert_it);

  // Insert record values into all column vectors
  RecordID id = count() + 1;
  game_date_est.insert(game_date_est.begin() + insert_at, record.game_date_est);
  team_id_home.insert(team_id_home.begin() + insert_at, record.team_id_home);
  fg_pct_home.insert(fg_pct_home.begin() + insert_at, record.fg_pct_home);
  ft_pct_home.insert(ft_pct_home.begin() + insert_at, record.ft_pct_home);
  fg3_pct_home.insert(fg3_pct_home.begin() + insert_at, record.fg3_pct_home);
  pts_home.insert(pts_home.begin() + insert_at, record.pts_home);
  ast_home.insert(ast_home.begin() + insert_at, record.ast_home);
  reb_home.insert(reb_home.begin() + insert_at, record.reb_home);
  home_team_wins.insert(home_team_wins.begin() + insert_at,
                        record.home_team_wins);

  return id;
}

Record Data::get(RecordID id) {
  // lookup position of record in block
  if (id >= count()) {
    std::ostringstream ss;
    ss << "Data::get(): Invalid record id: " << id;
    throw std::runtime_error(ss.str());
  }
  uint8_t i = id;
  return {
      game_date_est[i], team_id_home[i], fg_pct_home[i],
      ft_pct_home[i],   fg3_pct_home[i], pts_home[i],
      ast_home[i],      reb_home[i],     static_cast<bool>(home_team_wins[i]),
  };
}

void Data::read(std::istream &in) {
  uint8_t size = 0;
  // read metadata
  in.read(reinterpret_cast<char *>(&size), sizeof(size));
  in.read(reinterpret_cast<char *>(&next_id), sizeof(BlockID));
  // read record data
  read_vec(in, game_date_est, size);
  read_vec(in, team_id_home, size);
  read_vec(in, fg_pct_home, size);
  read_vec(in, ft_pct_home, size);
  read_vec(in, fg3_pct_home, size);
  read_vec(in, pts_home, size);
  read_vec(in, ast_home, size);
  read_vec(in, reb_home, size);
  read_vec(in, home_team_wins, size);
}

void Data::write(std::ostream &out) const {
  // write metadata
  uint8_t size_ = count();
  out.write(reinterpret_cast<const char *>(&size_), sizeof(size_));
  out.write(reinterpret_cast<const char *>(&next_id), sizeof(BlockID));
  // write record data in columar fashion
  write_vec(game_date_est, out);
  write_vec(team_id_home, out);
  write_vec(fg_pct_home, out);
  write_vec(ft_pct_home, out);
  write_vec(fg3_pct_home, out);
  write_vec(pts_home, out);
  write_vec(ast_home, out);
  write_vec(reb_home, out);
  write_vec(home_team_wins, out);
}

Key Data::key() const {
  // data kept in asecending sorted order: min is first item
  return (KEY_COL.size() <= 0) ? std::numeric_limits<Key>::max()
                               : Record::to_key(KEY_COL[0]);
}

// Overload equality operator
bool Data::operator==(const Data &other) const {
  return this->count() == other.count();
  game_date_est == other.game_date_est &&team_id_home ==
      other.team_id_home &&fg_pct_home == other.fg_pct_home &&ft_pct_home ==
      other.ft_pct_home &&fg3_pct_home == other.fg3_pct_home &&pts_home ==
      other.pts_home &&ast_home == other.ast_home &&reb_home ==
      other.reb_home &&home_team_wins == other.home_team_wins &&next_id ==
      other.next_id;
}
