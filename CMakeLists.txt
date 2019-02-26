cmake_minimum_required(VERSION 3.1)

project(sysperf)
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(sysp src/util.c)
add_executable(memperf src/memperf.c)
target_link_libraries(memperf sysp)
add_executable(fileperf src/fileperf.c)
add_executable(unordered_map_perf src/unordered_map_perf.cc)
target_link_libraries(unordered_map_perf sysp)
