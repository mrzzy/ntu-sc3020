/*
 * SC3020
 * Project 1
 * Filesystem
 */

#include <cstddef>
#include <stdexcept>
#include <sys/statvfs.h>

/** Get & returns filesystem block size of current working directory */
size_t block_size() {
  struct statvfs stat;
  if (statvfs(".", &stat) == -1) {
    throw std::runtime_error("Failed to get FS block size.");
  }
  return stat.f_bsize;
}
