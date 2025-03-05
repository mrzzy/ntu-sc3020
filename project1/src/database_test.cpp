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

void test_query(std::string db_path, QueryMode mode) {
  // test query index
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
}

TEST(database_test, test_load_query) {
  // test load
  std::filesystem::path db_path =
      std::filesystem::temp_directory_path() / "sc3020_database_test";
  std::shared_ptr store =
      std::make_shared<SpyStore>(std::make_shared<DiskStore>(db_path));
  Database db(store);

  std::fstream games_tsv(std::filesystem::current_path() / "games.txt");
  const auto &[n_records, n_levels] = db.load(games_tsv);
  // count no. of lines and check it matches with no. of records
  games_tsv.clear();
  games_tsv.seekg(0);
  int n_lines = std::count(std::istreambuf_iterator<char>(games_tsv),
                           std::istreambuf_iterator<char>(), '\n');
  EXPECT_EQ(n_records, n_lines - 1);

  EXPECT_GT(store->counts[SpyOpWrite][BlockKindData], 0);
  EXPECT_GT(store->counts[SpyOpWrite][BlockKindBTreeNode], 0);

  // test query
  test_query(db_path, QueryModeScan);
  test_query(db_path, QueryModeIndex);

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
