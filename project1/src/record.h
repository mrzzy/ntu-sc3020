#include <cstdint>
#ifndef RECORD_H
#define RECORD_H 1
/*
 * SC3020
 * Project 1
 * Record
 */

#include <string>
#include <string_view>

using Key = int32_t;

/** Record to store games.txt data. */
class Record {
public:
  static constexpr std::string_view TSV_HEADER =
      "GAME_DATE_EST	TEAM_ID_home	PTS_home	FG_PCT_home	"
      "FT_PCT_home	FG3_PCT_home	AST_home	REB_home	"
      "HOME_TEAM_WINS";

  // field arrangement minimises padding due to alignment
  time_t game_date_est;
  uint32_t team_id_home;
  float fg_pct_home;
  float ft_pct_home;
  float fg3_pct_home;
  uint8_t pts_home;
  uint8_t ast_home;
  uint8_t reb_home;
  bool home_team_wins;

  /** Convert the given key value into a comparable key */
  static Key to_key(float value) {
    // convert floating point key value to int for reliable comparison
    return value * 1000000;
  }

  /** Get the key of the Record */
  Key key() const { return Record::to_key(fg_pct_home); }

  /** Read record from TSV row by position */
  static Record from_tsv(const std::string &tsvRow);

  /** Render record as TSV row string */
  std::string to_tsv() const;
  /** Define equality operator for comparison */
  bool operator==(const Record &other) const;
};
time_t parse_date(const std::string &dateStr);
std::string format_date(time_t date);

#endif /* ifndef RECORD_H */
