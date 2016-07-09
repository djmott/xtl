find_package(GTest)

if (NOT GTEST_FOUND)
  message(FATAL_ERROR "Google test framework not found")
endif()

enable_testing()

include_directories("${GTEST_INCLUDE_DIRS}")

add_library(dummy SHARED "dummy_library.cpp")
add_dependencies(dummy XTL)

set(TEST_SOURCE "${XTL_SOURCE_DIR}/tests/tests.cpp")

function(build_option NAME DESC)
  option(${NAME} "${DESC}" YES)
  add_definitions(-D${NAME}=${${NAME}})
endfunction()

build_option(TEST_CALLBACK "test xtd::callback")
build_option(TEST_CONCURRENT_HASH_MAP "test xtd::concurrent_hash_map")
build_option(TEST_DATA_CONVERT "test data conversion")
build_option(TEST_DYNAMIC_LIBRARY "test xtd::dynamic_library")
build_option(TEST_EVENT_TRACE "test event trace")
build_option(TEST_EXCEPTION "test xtd::exception")
build_option(TEST_EXCUTABLE "test xtd::executable")
build_option(TEST_LOGGING "test xtd::log")
build_option(TEST_META "test meta programming")
build_option(TEST_PARSE "test xtd::parse")
build_option(TEST_PATH "test xtd::path")
build_option(TEST_READ_WRITE_LOCK "test xtd::read_write_lock")
build_option(TEST_RPC "test xtd::rpc")
build_option(TEST_SOCKET "test xtd::socket")
build_option(TEST_SOURCE_LOCATION "test xtd::source_location")
build_option(TEST_SPIN_LOCK "test xtd::spin_lock")
build_option(TEST_STRING "test xtd::string")

if(XTD_HAS_UUID)
  build_option(TEST_UNIQUE_ID "test xtd::unique_id")
endif()

build_option(TEST_VAR "test xtd::var")

add_executable(unit_tests "${TEST_SOURCE}")
add_dependencies(unit_tests dummy)
target_link_libraries(unit_tests "${GTEST_BOTH_LIBRARIES}")
#add_test(NAME unit_tests COMMAND unit_tests)
GTEST_ADD_TESTS(unit_tests "" "${TEST_SOURCE}")

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0 -coverage -Wall -fprofile-arcs -ftest-coverage")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -O0 -coverage -Wall -W -fprofile-arcs -ftest-coverage")
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -lgcov -coverage -fprofile-arcs -ftest-coverage")

  add_executable(coverage_tests "${TEST_SOURCE}")
  add_dependencies(coverage_tests dummy)
  target_link_libraries(coverage_tests "${GTEST_BOTH_LIBRARIES}")
endif()
