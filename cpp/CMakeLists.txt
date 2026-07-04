cmake_minimum_required(VERSION 3.10)
project(CVSDDelayCore)

set(CMAKE_CXX_STANDARD 17)

# Library
add_library(CVSDDelayCore STATIC CVSDDelayCore.cpp)

# Test Harness
add_executable(test_harness test/test_harness.cpp)
target_link_libraries(test_harness PRIVATE CVSDDelayCore)
