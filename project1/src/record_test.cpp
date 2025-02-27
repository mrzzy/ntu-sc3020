/*
 * SC3020
 * Project 1
 * Record Tests
 */

#include "record.h"
#include <gtest/gtest.h>
#include <iostream>
#include <string>

TEST(record_test, test_from_to_tsv) {
  std::string tsv = "1/1/1970	1610612739	114	0.482	0.786	"
                    "0.313	22	37	1";
  Record record = Record::from_tsv(tsv);
  // Validate parsed values
  EXPECT_EQ(record.game_date_est, 0);
  EXPECT_EQ(record.team_id_home, 1610612739);
  EXPECT_EQ(record.pts_home, 114);
  EXPECT_FLOAT_EQ(record.fg_pct_home, 0.482);
  EXPECT_FLOAT_EQ(record.key(), 482000);
  EXPECT_FLOAT_EQ(record.ft_pct_home, 0.786);
  EXPECT_FLOAT_EQ(record.fg3_pct_home, 0.313);
  EXPECT_EQ(record.ast_home, 22);
  EXPECT_EQ(record.reb_home, 37);
  EXPECT_EQ(record.home_team_wins, true);

  EXPECT_EQ(record.to_tsv(), tsv);
  
  std::cout << "Record size:" << sizeof(Record) << std::endl;
}
