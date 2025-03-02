/*
 * SC3020
 * Project 1
 * Data Tests
 */

#include "data.h"
#include "fs.h"
#include "id.h"
#include <gtest/gtest.h>
#include <iostream>
#include <ostream>
#include <sstream>
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
  ASSERT_LE(block.fg_pct_home[0], block.fg_pct_home[1]);
  ASSERT_EQ(block.key(), record2.key());

  for (int i = block.capacity - 2; i > 0; i--) {
    block.insert(record1);
  }
  ASSERT_THROW(block.insert(record1), std::runtime_error);


  ASSERT_EQ(block.get(id1), record1);
  ASSERT_EQ(block.get(id2), record2);
  ASSERT_THROW(block.get(999), std::runtime_error);
}


TEST(data_test, test_write_read) {
  Data block;
  Record record = Record::from_tsv(
      "1/1/1970	1610612739	114	0.582	0.786	"
      "0.313	22	37	1");
  block.next_id = 222;
  for (int i = 0; i < block.capacity; i++) {
    block.insert(record);
  }

  // test write
  std::stringstream ss;
  block.write(ss);
  std::cout << "wrote block size:" << ss.tellp() << std::endl;
  ASSERT_LE(ss.tellp(), block_size());

  // test read from write
  Data read;
  read.read(ss);
  ASSERT_EQ(block, read);
}
