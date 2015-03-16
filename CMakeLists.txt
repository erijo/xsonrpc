cmake_minimum_required(VERSION 2.8)
project(xsonrpc)

# Version number
set(xsonrpc_version_major 0)
set(xsonrpc_version_minor 1)

# Compiler flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

# tinyxml2 (TODO: use installed if available)
include_directories("3pp/tinyxml2")

# integer_sequence for c++11 compiler
include_directories("3pp/integer_seq")

# Public headers
include_directories("include/xsonrpc")

# Main library
add_library(xsonrpc SHARED
  3pp/tinyxml2/tinyxml2.cpp
  src/dispatcher.cpp
  src/fault.cpp
  src/request.cpp
  src/response.cpp
  src/server.cpp
  src/util.cpp
  src/value.cpp
)

# TODO: detect microhttpd
target_link_libraries(xsonrpc microhttpd)

# Unit tests
enable_testing()
include_directories("3pp/catch")
add_executable(unittest
  test/dispatchertest.cpp
  test/main.cpp
  test/requesttest.cpp
  test/responsetest.cpp
  test/valuetest.cpp
)
target_link_libraries(unittest xsonrpc)
add_test(NAME unittest COMMAND unittest)

set(unittest_stamp
  "${CMAKE_CURRENT_BINARY_DIR}/unittest.stamp")
add_custom_command(OUTPUT "${unittest_stamp}"
  COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
  COMMAND ${CMAKE_COMMAND} -E touch "${unittest_stamp}"
  COMMENT "Running unit test"
  DEPENDS unittest)
add_custom_target(unittest_run ALL DEPENDS "${unittest_stamp}")

# Examples
add_executable(testserver
  examples/testserver.cpp
)
target_link_libraries(testserver xsonrpc)
