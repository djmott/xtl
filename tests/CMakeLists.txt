
enable_testing()

option(XTD_INSTALL_TESTS "Install tests?" FALSE)



add_library(dummy_library_test SHARED dummy_library.cpp)
target_link_libraries(dummy_library_test PRIVATE xtl gtest)

if(XTD_OS STREQUAL "XTD_OS_WINDOWS")
  if(XTD_COMPILER STREQUAL "XTD_COMPILER_MINGW")
    option(BUILD_TEST_COVERAGE "profile test coverage" TRUE)
  else()
    set(BUILD_TEST_COVERAGE FALSE)
  endif()
else()
  option(BUILD_TEST_COVERAGE "profile test coverage" TRUE)
endif()

set(TEST_SOURCE
  tests.cpp
  test_com.hpp
  test_btree.hpp
  test_callback.hpp
  test_concurrent_stack.hpp
  test_debug_help.hpp
  test_dynamic_library.hpp
  test_event_trace.hpp
  test_exception.hpp
  test_executable.hpp
  test_hash_map.hpp
  test_logging.hpp
  test_lru_cache.hpp
  test_mapped_file.hpp
  test_mapped_vector.hpp
  test_meta.hpp
  test_parse.hpp
  test_path.hpp
  test_process.hpp
  test_rw_lock.hpp
  test_recursive_spin_lock.hpp
  test_rpc.hpp
  test_shared_mem_obj.hpp
  test_socket.hpp
  test_source_location.hpp
  test_spin_lock.hpp
  test_string.hpp
  test_stack.hpp
  test_unique_id.hpp
  test_var.hpp
)

function(build_option NAME DESC)
  option(${NAME} "${DESC}" TRUE)
  add_definitions(-D${NAME}=${${NAME}})
endfunction()

build_option(TEST_COM "test xtd::com")
build_option(TEST_BTREE "test xtd::btree")
build_option(TEST_CALLBACK "test xtd::callback")
build_option(TEST_CONCURRENT_HASH_MAP "test xtd::concurrent::hash_map")
build_option(TEST_CONCURRENT_STACK "test xtd::concurrent::stack")
build_option(TEST_DEBUG_HELP "test xtd::windows::debug_help")
build_option(TEST_DYNAMIC_LIBRARY "test xtd::dynamic_library")
build_option(TEST_EVENT_TRACE "test event trace")
build_option(TEST_EXCEPTION "test xtd::exception")
build_option(TEST_EXCUTABLE "test xtd::executable")
build_option(TEST_LOGGING "test xtd::log")
build_option(TEST_LRU_CACHE "test xtd::lru_cache")
build_option(TEST_MAPPED_FILE "test xtd::mapped_file")
build_option(TEST_MAPPED_VECTOR "test xtd::mapped_vector")
build_option(TEST_META "test meta programming")
#build_option(TEST_PARSE "test xtd::parse")
build_option(TEST_PATH "test xtd::filesystem::path")
build_option(TEST_PROCESS "test xtd::process")
build_option(TEST_READ_WRITE_LOCK "test xtd::concurrent::rw_lock")
build_option(TEST_RECURSIVE_SPIN_LOCK "test xtd::concurrent::recursive_spin_lock")
#build_option(TEST_RPC "test xtd::rpc")
build_option(TEST_SHARED_MEM_OBJ "test xtd::shared_mem_obj")
build_option(TEST_SOCKET "test xtd::socket")
build_option(TEST_SOURCE_LOCATION "test xtd::source_location")
build_option(TEST_SPIN_LOCK "test xtd::concurrent::spin_lock")
build_option(TEST_STACK "test xtd::concurrent::stack")
build_option(TEST_STRING "test xtd::string")

if(XTD_HAS_UUID OR XTD_OS STREQUAL "XTD_OS_WINDOWS")
  build_option(TEST_UNIQUE_ID "test xtd::unique_id")
endif()

build_option(TEST_VAR "test xtd::var")

if(XTD_OS STREQUAL "XTD_OS_WINDOWS" AND (XTD_COMPILER STREQUAL "XTD_COMPILER_GNU" ))
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility=hidden -Wall -O3 -Wa,-mbig-obj" )
endif()

add_executable(unit_tests ${TEST_SOURCE})
add_test(NAME unit_tests COMMAND $<TARGET_FILE:unit_tests>)
add_dependencies(unit_tests dummy_library_test)
target_link_libraries(unit_tests PRIVATE xtl)

if("Debug" STREQUAL "${CMAKE_BUILD_TYPE}")
  target_link_libraries(unit_tests PRIVATE gtest)
else()
  target_link_libraries(unit_tests PRIVATE gtest)
endif()

if(XTD_OS STREQUAL "XTD_OS_WINDOWS" AND BUILD_TEST_COVERAGE)
  target_compile_options(unit_tests PRIVATE -coverage -fprofile-arcs -ftest-coverage)
  target_link_libraries(unit_tests PRIVATE gcov)
endif()

if(XTD_COMPILER STREQUAL "XTD_COMPILER_GCC" AND (XTD_HAS_FILESYSTEM OR XTD_HAS_EXP_FILESYSTEM))
  target_link_libraries(unit_tests PRIVATE stdc++fs)
endif()


