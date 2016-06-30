enable_testing()

include_directories("${GTEST_INCLUDE_DIRS}")

add_library(dummy SHARED "dummy_library.cpp")
add_dependencies(dummy XTL)

set(TEST_SOURCE "${XTL_SOURCE_DIR}/tests/tests.cpp")

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
