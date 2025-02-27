/*
 * SC3020
 * Project 1
 * Data Tests
 */

#include "data.h"
#include "id.h"
#include <gtest/gtest.h>
#include <string>

TEST(data_test, test_insert_get) {
  Data block;
  Record record1 = Record::from_tsv(
      "1/1/1970	1610612739	114	0.582	0.786	"
      "0.313	22	37	1");
  RecordID id1 = block.insert(record1);
  Record record2 = Record::from_tsv(
      "1/1/1970	1610612739	114	0.482	0.786	"
      "0.313	22	37	1");
  RecordID id2 = block.insert(record2);
  ASSERT_NE(id1, id2);
  
  // check data stored in sorted order
  EXPECT_LE(block.fg_pct_home[0], block.fg_pct_home[1]);

  ASSERT_EQ(block.get(id1), record1);
  ASSERT_EQ(block.get(id2), record2);
}
