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
#include <ostream>
#include <sstream>
#include <stdexcept>
#include <vector>

Data::Data() {
  // determine data block record capacity from fs block size
  // header: 1 uint8_t storing number of records in block
  size_t header_size = sizeof(uint8_t);
  // need to store both record and record id mapping for each record
  size_t record_size = Record::size() + sizeof(RecordID);
  capacity = (block_size() - header_size) / record_size;
}

template <typename T>
void read_vec(std::istream &in, std::vector<T> &vec, uint8_t size) {
  // allocate space in vector for items
  vec.resize(size);
  in.read(reinterpret_cast<char *>(vec.data()), sizeof(T) * size);
}
RecordID Data::insert(const Record &record) {
  // reject inserts exceeding capacity
  if (count() >= capacity) {
    throw std::runtime_error("Data::insert(): insert exceeds block capacity");
  }

  // locate insertion position: find last occurrence of the records key
  auto insert_it = std::upper_bound(
      fg_pct_home.begin(), fg_pct_home.end(), record.key(),
      [](Key key, float value) { return Record::to_key(value) <= key; });
  auto insert_at = std::distance(fg_pct_home.begin(), insert_it);

  // Insert record values into all column vectors
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

  // shift record_pos mapping for moved position of all records following newly
  // inserted record
  for (int i = 0; i < record_pos.size(); i++) {
    if (record_pos[i] >= insert_at) {
      record_pos[i]++;
    }
  }
  // add record_pos mapping for newly inserted record
  RecordID id = record_pos.size();
  record_pos.push_back(insert_at);

  return id;
}

Record Data::get(RecordID id) {
  // lookup position of record in block
  if (id < 0 || id >= record_pos.size()) {
    std::ostringstream ss;
    ss << "Data::get(): Invalid record id: " << id;
    throw std::runtime_error(ss.str());
  }
  int i = record_pos[id];

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
  read_vec(in, record_pos, size);
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

/** Write vector as bytes to the given ostream */
template <typename T>
void write_vec(const std::vector<T> &vec, std::ostream &out) {
  out.write(reinterpret_cast<const char *>(vec.data()), vec.size() * sizeof(T));
}

void Data::write(std::ostream &out) const {
  // write metadata: header & record_pos map
  uint8_t size_ = count();
  out.write(reinterpret_cast<const char *>(&size_), sizeof(size_));
  write_vec(record_pos, out);
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

// Overload equality operator
bool Data::operator==(const Data &other) const {
  return this->count() == other.count();
  record_pos == other.record_pos &&game_date_est ==
      other.game_date_est &&team_id_home == other.team_id_home &&fg_pct_home ==
      other.fg_pct_home &&ft_pct_home == other.ft_pct_home &&fg3_pct_home ==
      other.fg3_pct_home &&pts_home == other.pts_home &&ast_home ==
      other.ast_home &&reb_home == other.reb_home &&home_team_wins ==
      other.home_team_wins;
}
