/*
 * SC3020
 * Project 1
 * Data Block
 */

#include "data.h"
#include "id.h"
#include <algorithm>
#include <iterator>

RecordID Data::insert(const Record &record) {
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
  // look position of record in block
  int i = record_pos[id];

  return {
      game_date_est[i], team_id_home[i], fg_pct_home[i],
      ft_pct_home[i],   fg3_pct_home[i], pts_home[i],
      ast_home[i],      reb_home[i],     home_team_wins[i],
  };
}
