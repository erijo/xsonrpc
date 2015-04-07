cmake_minimum_required(VERSION 2.8.11)
project(xsonrpc)

# Version number
set(XSONRPC_VERSION_MAJOR 0)
set(XSONRPC_VERSION_MINOR 1)
set(XSONRPC_VERSION "${XSONRPC_VERSION_MAJOR}.${XSONRPC_VERSION_MINOR}")

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
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

# integer_sequence for c++11 compiler
include_directories("3pp/integer_seq")

# Public headers
include_directories("include/xsonrpc")

# Main library
add_subdirectory("src")

# Unit tests
enable_testing()
add_subdirectory("test")

# Examples
add_subdirectory("examples" EXCLUDE_FROM_ALL)
