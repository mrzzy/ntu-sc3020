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
#include <iostream>
#include <iterator>
#include <ostream>
#include <sstream>
#include <stdexcept>

Data::Data() {
  // determine data block record capacity from fs block size
  // header: 1 uint8_t storing number of records in block
  size_t header_size = sizeof(uint8_t);
  // need to store both record and record id mapping for each record
  size_t record_size = Record::size() + sizeof(RecordID);
  capacity = (block_size() - header_size) / record_size;
}

RecordID Data::insert(const Record &record) {
  // reject inserts exceeding capacity
  if (size() >= capacity) {
    throw std::runtime_error("Data::insert(): insert exceeds block capacity");
  }

  // locate insertion position: find last occurrence of the records key
  auto insert_it = std::upper_bound(
      fg_pct_home.begin(), fg_pct_home.end(), record.key(),
      [](float value, Key key) { return Record::to_key(value) <= key; });
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

/** Compute the size of a vector in bytes */
template <typename T> size_t n_bytes(const std::vector<T> &vec) {
  return vec.size() * sizeof(T);
}

void Data::write(std::ostream &out) const {
  // write metadata: header & record map
  uint8_t size_ = size();
  out.write(reinterpret_cast<const char *>(&size_), sizeof(uint8_t));
  out.write(reinterpret_cast<const char *>(record_pos.data()),
            n_bytes(record_pos));

  // write record data in columar fashion
  out.write(reinterpret_cast<const char *>(game_date_est.data()),
            n_bytes(game_date_est));
  out.write(reinterpret_cast<const char *>(team_id_home.data()),
            n_bytes(team_id_home));
  out.write(reinterpret_cast<const char *>(fg_pct_home.data()),
            n_bytes(fg_pct_home));
  out.write(reinterpret_cast<const char *>(ft_pct_home.data()),
            n_bytes(ft_pct_home));
  out.write(reinterpret_cast<const char *>(fg3_pct_home.data()),
            n_bytes(fg3_pct_home));
  out.write(reinterpret_cast<const char *>(pts_home.data()), n_bytes(pts_home));
  out.write(reinterpret_cast<const char *>(ast_home.data()), n_bytes(ast_home));
  out.write(reinterpret_cast<const char *>(reb_home.data()), n_bytes(reb_home));
  out.write(reinterpret_cast<const char *>(home_team_wins.data()),
            n_bytes(home_team_wins));
}
