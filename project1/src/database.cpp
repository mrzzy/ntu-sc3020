/*
 * SC3020
 * Project 1
 * Database
 */

#include "database.h"
#include "data.h"
#include "store.h"
#include <algorithm>
#include <iostream>
#include <memory>
#include <ostream>
#include <string>
#include <vector>

/* Read records from games_tsv, grouped by key into data blocks */
using Blocks = std::map<Key, std::vector<std::shared_ptr<Data>>>;
Blocks read_records(std::istream &games_tsv) {
  // skip header line
  std::string line;
  std::getline(games_tsv, line);
  // read records from games_tsv & pack them into data blocks by key
  Blocks blocks;
  while (std::getline(games_tsv, line)) {
    Record record = Record::from_tsv(line);
    Key key = record.key();

    if (blocks[key].empty() || blocks[key].back()->is_full()) {
      // create a new block to store record for key
      blocks[key].push_back(std::make_shared<Data>());
    }
    blocks[key].back()->insert(record);
  }

  return blocks;
}

void Database::load(std::istream &games_tsv) {
  Blocks blocks = read_records(games_tsv);

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
  index.bulk_load(key_pointers);
  store->persist();
}
