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

find_package(Threads)
find_package(Doxygen)

include(CheckIncludeFileCXX)
include(CheckLibraryExists)
include(FeatureSummary)
include(ExternalProject)

option(XTD_COLOR_MESSAGES "use color escape sequences in console compiler output" NO)
option(XTD_LOG_TARGET_CSV "write log output to CSV file" NO)
option(XTD_LOG_TARGET_COUT "write log output to std::cout" NO)
option(XTD_LOG_TARGET_XML "write log output to XML file" NO)
option(XTD_LOG_TARGET_SYSLOG "write log output to syslog file" NO)
option(XTD_LOG_TARGET_WINDBG "write log output to attached debugger" NO)
option(BUILD_EXAMPLES "Build examples" YES)
option(BUILD_TESTS "Build tests" YES)


check_library_exists(uuid uuid_generate "" XTD_HAS_LIBUUID)
check_include_file_cxx(codecvt XTD_HAS_CODECVT CACHE BOOLEAN FALSE)
check_include_file_cxx(filesystem XTD_HAS_FILESYSTEM CACHE BOOLEAN FALSE)
check_include_file_cxx(iconv.h XTD_HAS_ICONV_H CACHE BOOLEAN FALSE)
check_library_exists(iconv iconv_open "" XTD_HAS_ICONV_LIB)

#detect OS
if(CMAKE_SYSTEM_NAME MATCHES "Darwin")
  set(XTD_OS XTD_OS_OSX)
elseif(CMAKE_SYSTEM_NAME MATCHES "MSYS")
  set(XTD_OS XTD_OS_MSYS)
elseif(CMAKE_SYSTEM_NAME MATCHES "Linux")
  set(XTD_OS XTD_OS_LINUX)
elseif(CMAKE_SYSTEM_NAME MATCHES "CYGWIN")
  set(XTD_OS XTD_OS_CYGWIN)
elseif(CMAKE_SYSTEM_NAME MATCHES "Windows")
  if(MINGW)
    set(XTD_OS XTD_OS_MINGW)
  else()
    set(XTD_OS XTD_OS_WINDOWS)
  endif()
else()
  message(FATAL_ERROR "Platform not supported")
endif()

#detect compiler
if(MSVC)
  set(XTD_COMPILER XTD_COMPILER_MSVC)
elseif(BORLAND)
  set(XTD_COMPILER XTD_COMPILER_BCC)
elseif(WATCOM)
  set(XTD_COMPILER XTD_COMPILER_WCL)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  set(XTD_COMPILER XTD_COMPILER_GCC)
else()
  message(FATAL_ERROR "Compiler not supported : ${CMAKE_CXX_COMPILER_ID}")
endif()


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
  include/xtd/debug.hpp
  include/xtd/dynamic_library.hpp
  include/xtd/event_trace.hpp
  include/xtd/exception.hpp
  include/xtd/executable.hpp
  include/xtd/filesystem.hpp
  include/xtd/log.hpp
  include/xtd/meta.hpp
  include/xtd/parse.hpp
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

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

if(XTD_COMPILER MATCHES "XTD_COMPILER_GCC")
  target_compile_options(XTL PUBLIC -std=c++11 -Wall -Wextra)
endif()

link_libraries(XTL)

if(XTD_HAS_LIBUUID)
  link_libraries(uuid)
endif()

#set default character set and conversion
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

set(XTD_CHARSET "XTD_CHARSET_MBCS" CACHE STRING "default character set")
set_property(CACHE XTD_CHARSET PROPERTY STRINGS XTD_CHARSET_MBCS XTD_CHARSET_UNICODE)

if(XTD_CHARSET STREQUAL "XTD_CHARSET_MBCS")
  add_definitions(-DMBCS)
else()
  add_definitions(-DUNICODE)
endif()


#link libraries
if(XTD_OS MATCHES "XTD_OS_CYGWIN")
  link_libraries(dl pthread stdc++)
elseif(XTD_OS MATCHES "XTD_OS_MINGW")
  link_libraries(rpcrt4 ws2_32 pthread stdc++)
elseif(XTD_OS MATCHES "XTD_OS_LINUX")
  link_libraries(dl pthread stdc++)
elseif(XTD_OS MATCHES "XTD_OS_WINDOWS")
  link_libraries(rpcrt4)
endif()



configure_file(${CMAKE_CURRENT_SOURCE_DIR}/include/xtd/xtd.hpp.in ${CMAKE_CURRENT_SOURCE_DIR}/include/xtd/xtd.hpp)


if(DOXYGEN_FOUND)
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/docs/Doxyfile.in ${CMAKE_CURRENT_SOURCE_DIR}/docs/Doxyfile)
  add_custom_target(
    docs COMMAND "${DOXYGEN_EXECUTABLE}" Doxyfile
    COMMENT "Generating Doxygen API Documentation" VERBATIM
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/docs"
  )
endif()


if(BUILD_EXAMPLES)
  add_subdirectory(examples)
endif()


if(BUILD_TESTS)
  add_subdirectory(tests)
endif()


