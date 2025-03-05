#ifndef DATABASE_H
#define DATABASE_H 1
/*
 * SC3020
 * Project 1
 * Database
 */

#include "store.h"
#include <istream>
#include <memory>
#include "btree.h"

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

  /** Load the given games.txt TSV into the database */
  void load(std::istream &games_tsv);

  /**
   * Query for "FG_PCT_HOME" between 0.6 to 0.9 (inclusive) in the given query
   * mode. Returns the average of "FG_PCT_HOME" values queried.
   */
  double query(QueryMode mode) const;
};

#endif /* ifndef DATABASE_H */
