/*
 * SC3020
 * Project 1
 * Database Test
 */

#include "block.h"
#include "database.h"
#include "disk_store.h"
#include "spy_store.h"
#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>
#include <memory>

constexpr double TOLERANCE = 0.00000001;

TEST(database_test, test_load_query) {
  // test load
  std::filesystem::path db_path =
      std::filesystem::temp_directory_path() / "sc3020_database_test";
  std::shared_ptr store =
      std::make_shared<SpyStore>(std::make_shared<DiskStore>(db_path));
  Database db(store);

  std::ifstream games_tsv(std::filesystem::current_path() / "games.txt");
  EXPECT_EQ(db.load(games_tsv), 26651);

  EXPECT_GT(store->counts[SpyOpWrite][BlockKindData], 0);
  EXPECT_GT(store->counts[SpyOpWrite][BlockKindBTreeNode], 0);

  // test query
  std::shared_ptr store2 =
      std::make_shared<SpyStore>(std::make_shared<DiskStore>(db_path));
  Database db2(store2);
  // query range 0.6 <= key <= 0.9
  Key begin = Record::to_key(0.6);
  Key end = Record::to_key(0.9);
  std::vector<Record> records = db2.query(QueryModeIndex, begin, end);
  std::cout << "queried: " << records.size() << std::endl;
  EXPECT_GT(records.size(), 0);

  // expected value: 0.61.. computed from pandas
  EXPECT_NEAR(mean_fg_pct_home(records), 0.6178739495798318, TOLERANCE);
  std::filesystem::remove(db_path);
}

TEST(database_test, test_mean_fg_pct_home) {
  std::vector<Record> records = {
      {0, 0, 0.50, 0, 1, 0, 0, 0, 0}, // FG_PCT_home = 0.50
      {0, 0, 0.60, 0, 0, 1, 0, 0, 0}, // FG_PCT_home = 0.60
      {0, 0, 0.40, 1, 0, 0, 0, 0, 1}  // FG_PCT_home = 0.40
  };

  // Expected mean FG_PCT_home
  double expected_mean = (0.50 + 0.60 + 0.40) / 3;

  // Assert that function computes correct mean
  EXPECT_NEAR(mean_fg_pct_home(records), expected_mean, TOLERANCE);
}
