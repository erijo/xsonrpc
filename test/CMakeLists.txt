add_executable(unittest
  dispatchertest.cpp
  main.cpp
  requesttest.cpp
  responsetest.cpp
  utiltest.cpp
  valuetest.cpp
)

target_link_libraries(unittest xsonrpc)
target_include_directories(unittest PRIVATE
  "${PROJECT_SOURCE_DIR}/include/xsonrpc")
target_include_directories(unittest PRIVATE ${TINYXML2_INCLUDE_DIR})

# Catch (test framework)
include_directories("${PROJECT_SOURCE_DIR}/3pp/catch/include")

add_test(NAME unittest COMMAND unittest)

# Make unit test run again on failure
set(unittest_stamp
  "${CMAKE_CURRENT_BINARY_DIR}/unittest.stamp")
add_custom_command(OUTPUT "${unittest_stamp}"
  COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
  COMMAND ${CMAKE_COMMAND} -E touch "${unittest_stamp}"
  COMMENT "Running unit test"
  DEPENDS unittest
  VERBATIM)
add_custom_target(unittest_run ALL DEPENDS "${unittest_stamp}")
