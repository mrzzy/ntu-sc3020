#include <vector>
#ifndef DATABASE_H
#define DATABASE_H 1
/*
 * SC3020
 * Project 1
 * Database
 */

#include "btree.h"
#include "store.h"
#include <istream>
#include <memory>

/** Query execution mode. */
enum QueryMode {
  /* Scan all data blocks */
  QueryModeScan = 0,
  /* Utilise index to reduce blocks scanned. */
  QueryModeIndex = 1
};

/** Database to load & query games.txt data */
class Database {
public:
  // database backing block storage
  std::shared_ptr<Store> store;
  // b+tree index of stored data
  BTree index;

  /** Create Database that stores & reads its data in the given store */
  Database(std::shared_ptr<Store> store) : store(store), index(*store) {}

  /**
   * Load the given games.txt TSV into the database.
   * Returns the no. of records loaded & levels of btree.
   * */
  std::pair<size_t, size_t> load(std::istream &games_tsv);

  /**
   * Query for records with keys between begin & end (inclusive) in the given
   * query mode. Returns list of matching records.
   */
  std::vector<Record> query(QueryMode mode, Key begin, Key end) const;
};

/** Compute average of fg_pct_home from the given records */
double mean_fg_pct_home(const std::vector<Record> &records);

#endif /* ifndef DATABASE_H */
