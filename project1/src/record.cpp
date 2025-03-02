/*
 * SC3020
 * Project 1
 * Record
 */

#include "record.h"
#include <ctime>
#include <iomanip>
#include <sstream>
#include <vector>

/** Parse "DD/MM/YYYY" format into uint32_t epoch timestamp */
time_t parse_date(const std::string &dateStr) {
  char delim = '\0';
  int year = 0;
  int month = 0;
  std::tm tm = {};
  std::stringstream ss(dateStr);

  ss >> tm.tm_mday >> delim >> month >> delim >> year;
  // tm_mon is 0-indexed month
  tm.tm_mon = month - 1;
  // tm_year is years since 1900
  tm.tm_year = year - 1900;
  // parse in utc timezone
  return timegm(&tm);
}

/* format uint32_t date (epoch timestamp) aback to "DD/MM/YYYY" */
std::string format_date(time_t date) {
  // decode in utc timezone
  std::tm *tm = std::gmtime(&date);
  std::stringstream ss;
  // tm_mon is 0-indexed month
  // tm_year is years since 1900
  ss << tm->tm_mday << "/" << tm->tm_mon + 1 << "/" << tm->tm_year + 1900;
  return ss.str();
}

Record Record::from_tsv(const std::string &tsvRow) {
  std::stringstream ss(tsvRow);
  std::string token;
  std::vector<std::string> fields;

  // Split TSV row into fields
  while (std::getline(ss, token, '\t')) {
    fields.push_back(token);
  }
  if (fields.size() != 9) {
    throw std::runtime_error("Invalid TSV row format: Expected 9 fields.");
  }

  // Convert and assign values
  Record r;
  // null handling: assign max value if empty
  r.game_date_est = fields[0].empty() ? std::numeric_limits<time_t>::max()
                                      : parse_date(fields[0]);
  r.team_id_home = fields[1].empty() ? std::numeric_limits<uint32_t>::max()
                                     : std::stoul(fields[1]);
  r.pts_home = fields[2].empty() ? std::numeric_limits<uint8_t>::max()
                                 : std::stoi(fields[2]);
  r.fg_pct_home = fields[3].empty() ? std::numeric_limits<float>::max()
                                    : std::stof(fields[3]);
  r.ft_pct_home = fields[4].empty() ? std::numeric_limits<float>::max()
                                    : std::stof(fields[4]);
  r.fg3_pct_home = fields[5].empty() ? std::numeric_limits<float>::max()
                                     : std::stof(fields[5]);
  r.ast_home = fields[6].empty() ? std::numeric_limits<uint8_t>::max()
                                 : std::stoi(fields[6]);
  r.reb_home = fields[7].empty() ? std::numeric_limits<uint8_t>::max()
                                 : std::stoi(fields[7]);
  r.home_team_wins =
      fields[8].empty()
          ? true
          : (fields[8] == "1");

  return r;
}

std::string Record::to_tsv() const {
  std::stringstream ss;
  ss << format_date(game_date_est) << "\t" << team_id_home << "\t"
     << static_cast<int>(pts_home) << "\t" << std::fixed << std::setprecision(3)
     << fg_pct_home << "\t" << ft_pct_home << "\t" << fg3_pct_home << "\t"
     << static_cast<int>(ast_home) << "\t" << static_cast<int>(reb_home) << "\t"
     << (home_team_wins ? "1" : "0");
  return ss.str();
}

bool Record::operator==(const Record &other) const {
  return game_date_est == other.game_date_est &&
         team_id_home == other.team_id_home &&
         to_key(fg_pct_home) == to_key(other.fg_pct_home) &&
         to_key(ft_pct_home) == to_key(other.ft_pct_home) &&
         to_key(fg3_pct_home) == to_key(other.fg3_pct_home) &&
         pts_home == other.pts_home && ast_home == other.ast_home &&
         reb_home == other.reb_home && home_team_wins == other.home_team_wins;
}
