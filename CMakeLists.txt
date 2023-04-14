cmake_minimum_required(VERSION 3.5.1)

# project name
# set to use C compiler
project(bcvm C CXX)

set(CMAKE_C_COMPILER /usr/bin/gcc)

set(EXEC bcvm)

# I../include
# L../lib
include_directories(include)
include_directories(src)

add_subdirectory(src)
link_directories(lib)

