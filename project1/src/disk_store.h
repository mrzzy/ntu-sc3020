#include "block.h"
#include "data.h"
#include "metadata.h"
#include <cstddef>
#include <fstream>
#ifndef DISK_STORE_H
/*
 * SC3020
 * Project 1
 * Disk Store
 */

#include "mem_store.h"
#include <memory>
#include <filesystem>

/** Block storage in a disk file */
class DiskStore : public Store {
public:
  // IO stream to read / write blocks to to disk.
  std::fstream file;
  // Buffer used to defer reading / writing to disk.
  std::unique_ptr<char[]> buffer;
  // Metadata block
  std::shared_ptr<Metadata> meta;

  /** Create disk store that stores data at path */
  DiskStore(const std::filesystem::path &path);

  /** Compute storage position of the given block id */
  size_t position(BlockID block_id) const;
  /** Flush data stored to disk */
  void flush();
  /** Close the disk store */
  void close();

  virtual BlockID insert(std::shared_ptr<Block> block) override;
  virtual void update(BlockID block_id, std::shared_ptr<Block> block) override;
  virtual std::shared_ptr<Block> get_block(BlockID block_id) override;
  virtual std::shared_ptr<Metadata> get_meta() const override {
      return meta;
  }
  virtual void set_meta(std::shared_ptr<Metadata> metadata) override;

};

#define DISK_STORE_H 1
#endif /* ifndef DISK_STORE_H */
