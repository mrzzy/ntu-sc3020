cmake_minimum_required(VERSION 3.10)
project(sc3020_p1)

# export compile_commands.json
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
# c++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_ENABLE_EXPORTS ON)

# build library to share objects between main executable & test executable
add_library(sc3020_p1_lib OBJECT
  fs.h
  fs.cpp
  record.h
  record.cpp
  block.h
  data.h
  data.cpp
  btree_node.h
  btree_node.cpp
  btree.h
  btree.cpp
  store.h
  mem_store.h
  mem_store.cpp
)

# build main executable
add_executable(sc3020_p1 main.cpp)
target_link_libraries(sc3020_p1 sc3020_p1_lib)

# Unit testing
# setup googletest as test runner
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/3fbe4db9a39291ae8d7a9c5f1d75896bb4c5a18f.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)
enable_testing()
add_executable(
  sc3020_p1_test
  # test files
  fs_test.cpp
  record_test.cpp
  data_test.cpp
  btree_node_test.cpp
  btree_test.cpp
  mem_store_test.cpp
)
target_link_libraries(
  sc3020_p1_test
  sc3020_p1_lib
  GTest::gtest_main
)
include(GoogleTest)
gtest_discover_tests(sc3020_p1_test)
