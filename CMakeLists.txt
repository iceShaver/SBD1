cmake_minimum_required(VERSION 3.12)
project(SBD_1)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -pedantic -Wextra")
add_executable(SBD_1 main.cpp record.cc record.hh buffer.hh records_generator.cc records_generator.hh sorter.cc sorter.hh config.hh)