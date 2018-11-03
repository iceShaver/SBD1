cmake_minimum_required(VERSION 3.12)
project(SBD_1)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -pedantic -Wextra")
find_package(Boost REQUIRED COMPONENTS program_options)
add_executable(SBD_1 main.cpp record.cc record.hh buffer.hh records_generator.hh sorter.hh config.hh terminal.hh measurements.hh tools.hh)
target_link_libraries(SBD_1 Boost::program_options)
target_link_libraries(SBD_1 -lstdc++fs)