project(.)

set(CMAKE_CXX_STANDARD 17)

set(bptree-target .)

aux_source_directory(./src bptree-src)
aux_source_directory(. bptree-src)

add_executable(bptree ${bptree-src})

target_include_directories(bptree PUBLIC ..)
target_include_directories(bptree PUBLIC include)

