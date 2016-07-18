
find_package(GTest)

if (NOT GTEST_FOUND)
  message(FATAL_ERROR "Google test framework not found")
endif()

enable_testing()

include_directories(${GTEST_INCLUDE_DIRS})

add_library(dummy_library_test SHARED dummy_library.cpp)
add_dependencies(dummy_library_test XTL)

set(TEST_SOURCE tests.cpp
  test_callback.hpp
  test_concurrent_stack.hpp
  test_debug_help.hpp
  test_dynamic_library.hpp
  test_event_trace.hpp
  test_exception.hpp
  test_executable.hpp
  test_hash_map.hpp
  test_logging.hpp
  test_meta.hpp
  test_parse.hpp
  test_path.hpp
  test_process.hpp
  test_rw_lock.hpp
  test_rpc.hpp
  test_shared_mem_obj.hpp
  test_socket.hpp
  test_source_location.hpp
  test_spin_lock.hpp
  test_string.hpp
  test_unique_id.hpp
  test_var.hpp
)

function(build_option NAME DESC)
  option(${NAME} "${DESC}" YES)
  add_definitions(-D${NAME}=${${NAME}})
endfunction()

build_option(TEST_CALLBACK "test xtd::callback")
build_option(TEST_CONCURRENT_HASH_MAP "test xtd::concurrent_hash_map")
build_option(TEST_CONCURRENT_STACK "test xtd::concurrent_stack")
build_option(TEST_DEBUG_HELP "test xtd::windows::debug_help")
build_option(TEST_DYNAMIC_LIBRARY "test xtd::dynamic_library")
build_option(TEST_EVENT_TRACE "test event trace")
build_option(TEST_EXCEPTION "test xtd::exception")
build_option(TEST_EXCUTABLE "test xtd::executable")
build_option(TEST_LOGGING "test xtd::log")
build_option(TEST_META "test meta programming")
build_option(TEST_PARSE "test xtd::parse")
build_option(TEST_PATH "test xtd::path")
build_option(TEST_PROCESS "test xtd::process")
build_option(TEST_READ_WRITE_LOCK "test xtd::read_write_lock")
build_option(TEST_RPC "test xtd::rpc")
build_option(TEST_SHARED_MEM_OBJ "test xtd::shared_mem_obj")
build_option(TEST_SOCKET "test xtd::socket")
build_option(TEST_SOURCE_LOCATION "test xtd::source_location")
build_option(TEST_SPIN_LOCK "test xtd::concurrent::spin_lock")
build_option(TEST_STRING "test xtd::string")

if(XTD_HAS_UUID)
  build_option(TEST_UNIQUE_ID "test xtd::unique_id")
endif()

build_option(TEST_VAR "test xtd::var")

add_executable(unit_tests ${TEST_SOURCE})
add_dependencies(unit_tests dummy_library_test)
target_link_libraries(unit_tests ${GTEST_BOTH_LIBRARIES})
#add_test(NAME unit_tests COMMAND unit_tests)
GTEST_ADD_TESTS(unit_tests "" ${TEST_SOURCE})

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  add_executable(coverage_tests ${TEST_SOURCE})
  target_compile_options(coverage_tests PRIVATE -coverage -fprofile-arcs -ftest-coverage)
  add_dependencies(coverage_tests dummy_library_test)
  target_link_libraries(coverage_tests gcov ${GTEST_BOTH_LIBRARIES})
endif()
