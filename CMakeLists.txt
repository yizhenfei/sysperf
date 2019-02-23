cmake_minimum_required(VERSION 3.1)

project(sysperf)
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_library(sysp src/util.c)
add_executable(fileperf src/fileperf.c)

