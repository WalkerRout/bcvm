cmake_minimum_required(VERSION 3.5.1)

# project name
# set to use C compiler
project(bcvm C CXX)


if(WIN32)
  set(CCompilerPath "C:/TDM-GCC-64/bin/gcc")
else()
  set(CCompilerPath "/usr/bin/gcc")
endif()

set(CMAKE_C_COMPILER ${CCompilerPath})

set(EXEC bcvm)

# I../include
# L../lib
include_directories(include)
include_directories(src)

add_subdirectory(src)
link_directories(lib)

