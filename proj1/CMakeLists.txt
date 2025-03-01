cmake_minimum_required(VERSION 3.13)


SET(CMAKE_CXX_COMPILER "/usr/bin/g++")
SET(CMAKE_C_COMPILER "/usr/bin/cc")

project(bptree)

enable_testing()

add_subdirectory(bptree)
add_subdirectory(test)