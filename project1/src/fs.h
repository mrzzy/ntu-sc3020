#ifndef FS_H
#define FS_H 1
/*
 * SC3020
 * Project 1
 * Filesystem
*/
#include <cstdint>
#include <vector>
#include <istream>

/** Get & returns filesystem block size of current working directory */
uint32_t block_size();

/** 
 * Read vector with size elements element type T from given stream in. 
 * Assumes items are packed sequentially in read stream.
*/
template <typename T>
void read_vec(std::istream &in, std::vector<T> &vec, size_t size) {
  // allocate space in vector for items
  vec.resize(size);
  in.read(reinterpret_cast<char *>(vec.data()), sizeof(T) * size);
}


/** Write vector as bytes to the given ostream.
 * Writes elements sequentially one by one into the stream. */
template <typename T>
void write_vec(const std::vector<T> &vec, std::ostream &out) {
  out.write(reinterpret_cast<const char *>(vec.data()), vec.size() * sizeof(T));
}

#endif /* ifndef FS_H */
