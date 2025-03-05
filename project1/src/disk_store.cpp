/*
 * SC3020
 * Project 1
 * Disk Store
 */

#include "disk_store.h"
#include "block.h"
#include "btree_node.h"
#include "fs.h"
#include <filesystem>
#include <fstream>
#include <memory>
#include <stdexcept>

constexpr size_t METADATA_POS = 0;

DiskStore::DiskStore(const std::filesystem::path &path)
    : buffer(new char[block_size()]), meta(std::make_shared<Metadata>()) {
  // open or create a new file on disk
  if (std::filesystem::exists(path)) {
    // existing file: read existing data
    file.open(path, std::ios::in | std::ios::out);

    // read metadata block from disk
    file.clear();
    file.seekg(METADATA_POS);
    meta->read(file);
  } else {
    file.open(path, std::ios::in | std::ios::out | std::ios::trunc);
  }

  // buffer io stream with explicitly sized buffer
  file.rdbuf()->pubsetbuf(buffer.get(), block_size());
}

size_t DiskStore::position(BlockID block_id) const {
  // +1 reserved space for metadata block
  return (block_id + 1) * block_size();
}

BlockID DiskStore::insert(std::shared_ptr<Block> block) {
  // register block in metadata
  BlockID id = register_id(block);
  // write block to correct position for that block id
  file.clear();
  file.seekp(position(id));
  block->write(file);
  return id;
}
// Update an existing block by BlockID
void DiskStore::update(BlockID block_id, std::shared_ptr<Block> block) {
  // overwrite block to correct position for that block id
  file.clear();
  file.seekp(position(block_id));
  block->write(file);
}

// Retrieve a block by its BlockID
std::shared_ptr<Block> DiskStore::get_block(BlockID block_id) {
  // lookup type of block by id
  BlockKind kind = get_meta()->lookup(block_id);

  // read block from correct position for that block id
  file.clear();
  file.seekg(position(block_id));
  if (kind == BlockKindData) {
    std::shared_ptr block = std::make_shared<Data>();
    block->read(file);
    return block;
  } else if (kind == BlockKindBTreeNode) {
    std::shared_ptr block = std::make_shared<BTreeNode>();
    block->read(file);
    return block;
  }
  throw std::runtime_error("Block::read: Unsupported block type");
}

void DiskStore::set_meta(std::shared_ptr<Metadata> metadata) {
  meta = metadata;
}

void DiskStore::persist() {
  // persist metadata changes to disk
  file.clear();
  file.seekp(METADATA_POS);
  meta->write(file);

  file.flush();
}
