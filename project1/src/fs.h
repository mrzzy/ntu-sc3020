#ifndef FS_H
#define FS_H 1
/*
 * SC3020
 * Project 1
 * Filesystem
*/
#include <cstdint>
#include <filesystem>

/** Get & returns filesystem block size of current working directory */
uint32_t block_size();

#endif /* ifndef FS_H */
