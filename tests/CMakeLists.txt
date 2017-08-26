
include(FindGTest)

enable_testing()

if(GTEST_FOUND)
  link_libraries(${GTEST_BOTH_LIBRARIES})
  include_directories(${GTEST_INCLUDE_DIRS})
else()
  #include_directories(${ASSET_DIR}/gtest/googletest/include)
  if(XTD_COMPILER STREQUAL "XTD_COMPILER_MINGW")
    set(gtest_disable_pthreads ON CACHE BOOL "" FORCE)
  endif()
  if(XTD_COMPILER STREQUAL "XTD_COMPILER_MINGW" OR XTD_COMPILER STREQUAL "XTD_COMPILER_MSVC")
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
  endif()
  ExternalProject_ADD(gtest
    GIT_REPOSITORY https://github.com/google/googletest.git
    PREFIX gtest
    INSTALL_COMMAND ""
  )
  link_directories(${CMAKE_CURRENT_BINARY_DIR}/gtest/src/gtest-build/googlemock/gtest)
  include_directories(${CMAKE_CURRENT_BINARY_DIR}/gtest/src/gtest/googletest/include)
  link_libraries(gtest gtest_main)
endif()



add_library(dummy_library_test SHARED dummy_library.cpp)
add_dependencies(dummy_library_test XTL)

if(XTD_OS STREQUAL "XTD_OS_WINDOWS")
  if(XTD_COMPILER STREQUAL "XTD_COMPILER_MINGW")
    option(BUILD_TEST_COVERAGE "profile test coverage" ON)
  else()
    set(BUILD_TEST_COVERAGE OFF)
  endif()
else()
  option(BUILD_TEST_COVERAGE "profile test coverage" ON)
endif()

set(TEST_SOURCE tests.cpp
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
  option(${NAME} "${DESC}" YES)
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
build_option(TEST_PARSE "test xtd::parse")
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

add_executable(unit_tests ${TEST_SOURCE})
add_dependencies(unit_tests dummy_library_test)

if(BUILD_TEST_COVERAGE)
  target_compile_options(unit_tests PRIVATE -coverage -fprofile-arcs -ftest-coverage)
  target_link_libraries(unit_tests gcov)
endif()


if(GTEST_FOUND)
  GTEST_ADD_TESTS(unit_tests "" ${TEST_SOURCE})
else()
  add_test(unit_tests "" ${TEST_SOURCE})
  add_dependencies(unit_tests gtest)
endif()

if(XTD_COMPILER STREQUAL "XTD_COMPILER_GCC" AND (XTD_HAS_FILESYSTEM OR XTD_HAS_EXP_FILESYSTEM))
    target_link_libraries(unit_tests stdc++fs)
endif()
