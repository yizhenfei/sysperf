cmake_minimum_required(VERSION 2.8)

project(sysperf)
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_executable(diskperf src/diskperf.c)
