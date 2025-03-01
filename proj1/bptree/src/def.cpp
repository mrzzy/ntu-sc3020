//
// Created on 2025/2/20.
//
#include "def.h"

std::ostream &operator<<(std::ostream &os, const RecordKey &key) {
    os << "RecKey{id:" << key.id << ", fg3_pct_home:" << key.fg3_pct_home << "}";
    return os;
}

std::ostream &operator<<(std::ostream &os, const TeamsRecord &record) {
    os << "TeamsRecord{id:" << record.key
       << ", game_data_set:" << record.game_date_set
       << ", team_id_home:" << record.team_id_home
       << ", pts_home:" << record.pts_home
       << ", fg_pct_home:" << record.fg_pct_home
       << ", ft_pct_home:" << record.ft_pct_home
       << ", ast_home:" << record.ast_home
       << ", reb_home:" << record.reb_home
       << ", home_team_wins:" << record.home_team_wins
       << "}";
    return os;
}
