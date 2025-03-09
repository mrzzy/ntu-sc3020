/*
 * SC3020
 * Project 1
 * Entrypoint
 */
#include <chrono>
#include <fstream>
#include <iostream>
#include <memory>
#include <ostream>

#include "block.h"
#include "data.h"
#include "database.h"
#include "disk_store.h"
#include "spy_store.h"

int main(int argc, char *argv[]) {
  // parse args
  if (argc < 1 + 3) {
    std::cout << "Fatal: Expected 3 arguments" << std::endl;
    std::cout << "Usage:" << std::endl;
    std::cout << "sc3020_p1 load <db_path> <games.txt> \t Load games.txt into "
                 "database file at 'db_path'"
              << std::endl;
    std::cout << "sc3020_p1 query <db_path> <scan|index> \t Perform query on "
                 "database file at 'db_path' using scan or index query mode."
              << std::endl;
    return 1;
  }
  std::string command(argv[1]);
  std::string db_path(argv[2]);

  // setup database
  std::shared_ptr store =
      std::make_shared<SpyStore>(std::make_shared<DiskStore>(db_path));
  Database db(store);

  if (command == "load") {
    // load database from games.txt
    std::fstream games_txt(argv[3]);
    const auto &[n_records, n_levels] = db.load(games_txt);

    // print statistics for tasks
    std::cout << "[Task 1]" << std::endl;
    std::cout << "Record size: " << Record::size() << std::endl;
    std::cout << "Loaded records: " << n_records << std::endl;
    std::cout << "Max records per data block:  "
              << unsigned(Data::fs_capacity()) << std::endl;
    std::cout << "No. of data blocks: "
              << store->get_meta()->get_ids(BlockKindData).size() << std::endl;

    std::cout << "[Task 2]" << std::endl;
    std::cout << "Keys per B+Tree node (parameter n): "
              << BTreeNode::fs_capacity() << std::endl;
    std::cout << "No. of B+Tree nodes: "
              << store->get_meta()->get_ids(BlockKindBTreeNode).size()
              << std::endl;
    std::cout << "Levels in B+Tree: " << n_levels << std::endl;
    // dump root node keys
    std::cout << "Keys in root node:";
    std::shared_ptr root = store->get<BTreeNode>(db.index.root());
    for (const Key &key : root->keys) {
      std::cout << " " << key;
    }
    std::cout << std::endl;
  } else if (command == "query") {
    std::string mode(argv[3]);
    QueryMode query_mode;
    // parse query mode
    if (mode == "index") {
      query_mode = QueryModeIndex;
    } else if (mode == "scan") {
      query_mode = QueryModeScan;
    } else {
      std::cout << "Fatal: unsupported query mode: " << mode << std::endl;
      return 1;
    }

    // perform query: measuring execution time
    auto begin_at = std::chrono::high_resolution_clock::now();

    std::vector<Record> records =
        db.query(query_mode, Record::to_key(0.6), Record::to_key(0.9));
    double average = mean_fg_pct_home(records);

    auto end_at = std::chrono::high_resolution_clock::now();
    int duration =
        std::chrono::duration_cast<std::chrono::microseconds>(end_at - begin_at)
            .count();

    // print result & statistics for tasks
    std::cout << "[Task 3]" << std::endl;
    std::cout << "Index block accesses: "
              << store->counts[SpyOpRead][BlockKindBTreeNode] << std::endl;
    std::cout << "Data block accesses: "
              << store->counts[SpyOpRead][BlockKindData] << std::endl;
    std::cout << "Query result: " << average << std::endl;
    std::cout << "Query time (microseconds): " << duration << std::endl;
  } else {
    std::cout << "Fatal: unsupported command: " << command << std::endl;
    return 1;
  }
}
