#https://cmake.org/cmake/help/v2.8.12/cmake.html
#cmake config

set(CMAKE_LEGACY_CYGWIN_WIN32 0)
set(CMAKE_CXX_STANDARD 11)

cmake_minimum_required(VERSION 2.8.12)
if(POLICY CMP0054)
  cmake_policy(SET CMP0054 NEW)
endif()
if(POLICY CMP0056)
  cmake_policy(SET CMP0056 NEW)
endif()

enable_language(CXX)

project(XTL)

option(XTD_COLOR_MESSAGES "use color escape sequences in console compiler output" YES)

find_package(Threads)
find_package(Doxygen)

include(CheckIncludeFileCXX)
include(CheckLibraryExists)
include(FeatureSummary)
include(ExternalProject)

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEBUG")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} -DDEBUG")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -DNDEBUG")
set(CMAKE_CXX_FLAGS_MINSIZEREL "${CMAKE_CXX_FLAGS_MINSIZEREL} -DNDEBUG")



add_library(XTL STATIC src/xtl.cpp
  src/event_trace/event_trace.cpp
  include/xtd/callback.hpp
  include/xtd/concurrent.hpp
  include/xtd/concurrent_hash_map.hpp
  include/xtd/concurrent_stack.hpp
  include/xtd/data_convert.hpp
  include/xtd/debug.hpp
  include/xtd/dynamic_library.hpp
  include/xtd/event_trace.hpp
  include/xtd/exception.hpp
  include/xtd/executable.hpp
  include/xtd/log.hpp
  include/xtd/meta.hpp
  include/xtd/parse.hpp
  include/xtd/path.hpp
  include/xtd/process.hpp
  include/xtd/rw_lock.hpp
  include/xtd/rpc.hpp
  include/xtd/shared_mem_obj.hpp
  include/xtd/socket.hpp
  include/xtd/source_location.hpp
  include/xtd/spin_lock.hpp
  include/xtd/string.hpp
  include/xtd/tuple.hpp
  include/xtd/unique_id.hpp
  include/xtd/var.hpp
  include/xtd/xtd.hpp
  include/xtd/xtd.hpp.in
)

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  target_compile_options(XTL PUBLIC -std=c++11)
endif()

link_libraries(XTL)

check_library_exists(uuid uuid_generate "" XTD_HAS_LIBUUID)

if(XTD_HAS_LIBUUID)
  link_libraries(uuid)
endif()

check_include_file_cxx(codecvt XTD_HAS_CODECVT CACHE BOOLEAN FALSE)
check_include_file_cxx(iconv.h XTD_HAS_ICONV_H CACHE BOOLEAN FALSE)
check_library_exists(iconv iconv_open "" XTD_HAS_ICONV_LIB)


set(XTD_STR_CONVERT XTD_STR_CONVERT_GENERIC CACHE STRING "default string conversion backend")
set_property(CACHE XTD_STR_CONVERT PROPERTY STRINGS XTD_STR_CONVERT_GENERIC XTD_STR_CONVERT_CODECVT XTD_STR_CONVERT_ICONV)

if(XTD_HAS_CODECVT)
  set(XTD_STR_CONVERT XTD_STR_CONVERT_CODECVT)
elseif(XTD_HAS_ICONV_H AND XTD_HAS_ICONV_LIB)
  set(XTD_STR_CONVERT XTD_STR_CONVERT_ICONV)
  link_libraries(iconv)
else()
  set(XTD_STR_CONVERT XTD_STR_CONVERT_GENERIC)
endif()


if(MSVC)
  set(XTD_COMPILER XTD_COMPILER_MSVC)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  set(XTD_COMPILER XTD_COMPILER_GCC)
else()
  message(FATAL_ERROR "Compiler not supported : ${CMAKE_CXX_COMPILER_ID}")
endif()

set(XTD_CHARSET "XTD_CHARSET_MBCS" CACHE STRING "default character set")
set_property(CACHE XTD_CHARSET PROPERTY STRINGS XTD_CHARSET_MBCS XTD_CHARSET_UNICODE)

if(XTD_CHARSET STREQUAL "XTD_CHARSET_MBCS")
  add_definitions(-DMBCS)
else()
  add_definitions(-DUNICODE)
endif()


option(XTD_LOG_TARGET_CSV "write log output to CSV file" YES)
option(XTD_LOG_TARGET_COUT "write log output to std::cout" YES)
option(XTD_LOG_TARGET_XML "write log output to XML file" YES)


if(CYGWIN)
  set(XTD_OS XTD_OS_CYGWIN)
  option(XTD_LOG_TARGET_SYSLOG "write log output to syslog file" NO)
  link_libraries(dl pthread stdc++)
elseif(MINGW)
  set(XTD_OS XTD_OS_MINGW)
  option(XTD_LOG_TARGET_WINDBG "write log output to attached debugger" NO)
  link_libraries(rpcrt4 ws2_32 pthread stdc++)
elseif(WIN32)
  set(XTD_OS XTD_OS_WINDOWS)
  option(XTD_LOG_TARGET_WINDBG "write log output to attached debugger" NO)
  link_libraries(rpcrt4)
elseif(APPLE)
  set(XTD_OS XTD_OS_APPLE)
elseif(UNIX)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
  set(XTD_OS XTD_OS_LINUX)
  option(XTD_LOG_TARGET_SYSLOG "write log output to syslog file" NO)
  link_libraries(dl pthread stdc++)
else()
  message(FATAL_ERROR "system not supported")
endif()


include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/include/xtd/xtd.hpp.in ${CMAKE_CURRENT_SOURCE_DIR}/include/xtd/xtd.hpp)


if(DOXYGEN_FOUND)
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/docs/Doxyfile.in ${CMAKE_CURRENT_SOURCE_DIR}/docs/Doxyfile)
  add_custom_target(
    docs COMMAND "${DOXYGEN_EXECUTABLE}" Doxyfile
    COMMENT "Generating Doxygen API Documentation" VERBATIM
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/docs"
  )
endif()


option(BUILD_EXAMPLES "Build examples" YES)
if(BUILD_EXAMPLES)
  add_subdirectory(examples)
endif()


option(BUILD_TESTS "Build tests" YES)
if(BUILD_TESTS)
  add_subdirectory(tests)
endif()


