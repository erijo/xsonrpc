cmake_minimum_required(VERSION 2.8.11)
project(xsonrpc)

# Version number
set(XSONRPC_VERSION_MAJOR 0)
set(XSONRPC_VERSION_MINOR 1)
set(XSONRPC_VERSION "${XSONRPC_VERSION_MAJOR}.${XSONRPC_VERSION_MINOR}")

# Options
option(BUILD_EXAMPLES "Build xsonrpc examples" ON)
option(BUILD_TESTS "Build xsonrpc unit tests" ON)

include(CheckCXXCompilerFlag)
include(FindPackageMessage)

# TinyXML-2
find_path(TINYXML2_INCLUDE_DIR tinyxml2.h)
mark_as_advanced(TINYXML2_INCLUDE_DIR)
find_library(TINYXML2_LIBRARY tinyxml2)
mark_as_advanced(TINYXML2_LIBRARY)

if (TINYXML2_INCLUDE_DIR AND TINYXML2_LIBRARY)
  set(TINYXML2_FOUND ON)
  find_package_message(TINYXML2 "Found TinyXML-2: ${TINYXML2_LIBRARY}"
    "[${TINYXML2_LIBRARY}][${TINYXML2_INCLUDE_DIR}]")
else()
  set(TINYXML2_SOURCE "${PROJECT_SOURCE_DIR}/3pp/tinyxml2/tinyxml2.cpp")
  set(TINYXML2_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/3pp/tinyxml2")
endif()

# libmicrohttpd
find_path(MICROHTTPD_INCLUDE_DIR microhttpd.h)
mark_as_advanced(MICROHTTPD_INCLUDE_DIR)
find_library(MICROHTTPD_LIBRARY microhttpd)
mark_as_advanced(MICROHTTPD_LIBRARY)

if (NOT MICROHTTPD_INCLUDE_DIR OR NOT MICROHTTPD_LIBRARY)
  message(SEND_ERROR "Required library microhttpd not found")
endif()

# cURL
find_package(CURL REQUIRED)

# Compiler flags
check_cxx_compiler_flag(-Wall COMPILER_SUPPORTS_WALL)
if (COMPILER_SUPPORTS_WALL)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
endif()
check_cxx_compiler_flag(-Wextra COMPILER_SUPPORTS_WEXTRA)
if (COMPILER_SUPPORTS_WEXTRA)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wextra")
endif()

check_cxx_compiler_flag(-std=c++14 COMPILER_SUPPORTS_CXX14)
if (NOT COMPILER_SUPPORTS_CXX14)
  check_cxx_compiler_flag(-std=c++11 COMPILER_SUPPORTS_CXX11)
endif()
if (COMPILER_SUPPORTS_CXX14)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")
elseif (COMPILER_SUPPORTS_CXX11)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
else()
  message(SEND_ERROR "Compiler does not support C++11")
endif()

# integer_sequence for c++11 compiler
if (NOT COMPILER_SUPPORTS_CXX14)
  include_directories("3pp/integer_seq")
endif()
install(FILES "3pp/integer_seq/integer_seq.h" DESTINATION "include/xsonrpc")

# Public headers
set(XSONRPC_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include")
install(DIRECTORY "include/xsonrpc" DESTINATION "include"
  FILES_MATCHING PATTERN "*.h")

# Main library
add_subdirectory("src")

# Unit tests
if (BUILD_TESTS)
  enable_testing()
  add_subdirectory("test")
endif()

# Examples
if (BUILD_EXAMPLES)
  add_subdirectory("examples")
else()
  add_subdirectory("examples" EXCLUDE_FROM_ALL)
endif()
