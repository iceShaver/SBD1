cmake_minimum_required(VERSION 3.12)
project(SBD_1)

set(CMAKE_CXX_STANDARD 17)

add_executable(SBD_1 main.cpp record.cc record.hh buffer.cc buffer.hh records_generator.cc records_generator.hh)