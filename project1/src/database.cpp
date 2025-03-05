/*
 * SC3020
 * Project 1
 * Database
 */

#include "database.h"
#include "btree.h"
#include "data.h"
#include <deque>
#include <iostream>
#include <memory>
#include <string>
#include <strings.h>
#include <unordered_set>
#include <vector>

std::pair<size_t, size_t> Database::load(std::istream &games_tsv) {
  // skip header line
  std::string line;
  std::getline(games_tsv, line);
  // read records from games_tsv & pack them into data blocks by key
  // this means each data block only records from 1 key
  std::map<Key, std::vector<std::shared_ptr<Data>>> blocks;
  size_t n_records = 0;
  while (std::getline(games_tsv, line)) {
    Record record = Record::from_tsv(line);
    Key key = record.key();

    if (blocks[key].empty() || blocks[key].back()->is_full()) {
      // create a new block to store record for key
      blocks[key].push_back(std::make_shared<Data>());
    }
    blocks[key].back()->insert(record);
    n_records++;
  }

  // write data blocks to store
  std::map<Key, BlockID> key_pointers;
  for (const auto &[key, blocks] : blocks) {
    BlockID block_id = BLOCK_NULL;
    // insert blocks in reverse order so that block id of next block would be
    // availble for linking
    for (int i = blocks.size() - 1; i >= 0; i--) {
      blocks[i]->next_id = block_id;
      block_id = store->insert(blocks[i]);
    }

    // collect first data block id for each for indexing
    key_pointers[key] = block_id;
  }
  // build B+tree index on collected key data block pointers
  // since each data block only contains 1 key the index built is dense
  size_t n_levels = index.bulk_load(key_pointers);
  store->persist();

  return std::make_pair(n_records, n_levels);
}

std::vector<Record> Database::query(QueryMode mode, Key begin, Key end) const {
  // collect data block_ids to scan based on query mode
  std::vector<BlockID> data_ids;
  if (mode == QueryModeScan) {
    // scan: all data blocks
    data_ids = store->get_meta()->data_ids;
  } else if (mode == QueryModeIndex) {
    // index: range query of matching data blocks
    data_ids = index.range(begin, end);
  }

  // scan data blocks for matching records
  std::deque<BlockID> scan_ids(data_ids.begin(), data_ids.end());
  std::unordered_set<BlockID> seen_ids;
  std::vector<Record> records;
  while (!scan_ids.empty()) {
    BlockID data_id = scan_ids.front();
    scan_ids.pop_front();

    if (seen_ids.count(data_id) >= 1) {
      // skip seen data bocks
      continue;
    }

    // read data block & scan records
    std::shared_ptr data = store->get<Data>(data_id);
    if (data->key() < begin || data->key() > end) {
      // data block does not match query criteria: skip
      continue;
    }

    // since all records within the same data block share the same key
    // extract all records from data block as matching records
    for (uint8_t i = 0; i < data->count(); i++) {
      records.push_back(data->get(i));
    }

    // queue next data block id in data block chain (if any) for processing
    if (data->next_id != BLOCK_NULL) {
      scan_ids.push_back(data->next_id);
    }

    // mark data block as seen
    seen_ids.insert(data_id);
  }

  return records;
}

double mean_fg_pct_home(const std::vector<Record> &records) {
  double sum = 0.0;
  for (const Record &record : records) {
    sum += record.fg_pct_home;
  }
  return sum / records.size();
}
