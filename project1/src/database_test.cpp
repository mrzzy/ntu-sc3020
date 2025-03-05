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

TEST(database_test, test_load) {
  std::filesystem::path db_path =
      std::filesystem::temp_directory_path() / "sc3020_database_test";
  std::shared_ptr store =
      std::make_shared<SpyStore>(std::make_shared<DiskStore>(db_path));
  Database db(store);

  std::ifstream games_tsv(std::filesystem::current_path() / "games.txt");
  db.load(games_tsv);

  EXPECT_GT(store->counts[SpyOpWrite][BlockKindData], 0);
  EXPECT_GT(store->counts[SpyOpWrite][BlockKindBTreeNode], 0);
  std::filesystem::remove(db_path);
}
