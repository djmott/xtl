#https://cmake.org/cmake/help/v2.8.12/cmake.html
#cmake config
cmake_minimum_required(VERSION 2.8.12)
if(POLICY CMP0054)
  cmake_policy(SET CMP0054 NEW)
endif()
if(POLICY CMP0056)
  cmake_policy(SET CMP0056 NEW)
endif()
set(CMAKE_LEGACY_CYGWIN_WIN32 0)
enable_language(CXX)
set(CMAKE_CXX_STANDARD 11)

project(XTL)

set(XTD_COLOR_MESSAGES TRUE CACHE BOOLEAN "use color escape sequences in console compiler output")
set(XTD_CI_BUILD FALSE CACHE BOOLEAN "build for CI")

find_package(Threads)
find_package(Doxygen)
find_package(GTest)

include(CheckIncludeFileCXX)
include(CheckLibraryExists)
include(FeatureSummary)
include(ExternalProject)

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEBUG")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} -DDEBUG")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -DNDEBUG")
set(CMAKE_CXX_FLAGS_MINSIZEREL "${CMAKE_CXX_FLAGS_MINSIZEREL} -DNDEBUG")



add_library(XTL STATIC  "${XTL_SOURCE_DIR}/src/xtl.cpp" "${XTL_SOURCE_DIR}/src/event_trace/event_trace.cpp")

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


set(XTD_LOG_TARGET_CSV TRUE CACHE BOOL "write log output to CSV file")
set(XTD_LOG_TARGET_COUT TRUE CACHE BOOL "write log output to std::cout")
set(XTD_LOG_TARGET_XML FALSE CACHE BOOL "write log output to XML file")


if(CYGWIN)
  set(XTD_OS XTD_OS_CYGWIN)
  set(XTD_LOG_TARGET_SYSLOG FALSE CACHE BOOL "write log output to syslog file")
  set(XTD_LOG_TARGET_WINDBG FALSE)
  link_libraries(dl pthread stdc++)
elseif(MINGW)
  set(XTD_OS XTD_OS_MINGW)
  set(XTD_LOG_TARGET_WINDBG FALSE CACHE BOOL "write log output to attached debugger")
  set(XTD_LOG_TARGET_SYSLOG FALSE)
  link_libraries(rpcrt4 ws2_32 pthread stdc++)
elseif(WIN32)
  set(XTD_OS XTD_OS_WINDOWS)
  set(XTD_LOG_TARGET_WINDBG FALSE CACHE BOOL "write log output to attached debugger")
  set(XTD_LOG_TARGET_SYSLOG FALSE)
  link_libraries(rpcrt4)
elseif(APPLE)
  set(XTD_OS XTD_OS_APPLE)
elseif(UNIX)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
  set(XTD_OS XTD_OS_UNIX)
  set(XTD_LOG_TARGET_SYSLOG FALSE CACHE BOOL "write log output to syslog file")
  set(XTD_LOG_TARGET_WINDBG FALSE)
  link_libraries(dl pthread stdc++)
else()
  message(FATAL_ERROR "system not supported")
endif()


include_directories("${XTL_BINARY_DIR}" "${XTL_SOURCE_DIR}/include")

configure_file("${XTL_SOURCE_DIR}/include/xtd/xtd.hpp.in" "${CMAKE_CURRENT_BINARY_DIR}/xtd.hpp")

if(DOXYGEN_FOUND)

  configure_file("${XTL_SOURCE_DIR}/Doxyfile.in" "${XTL_SOURCE_DIR}/Doxyfile")

  add_custom_target(
    docs COMMAND "${DOXYGEN_EXECUTABLE}" Doxyfile
    COMMENT "Generating Doxygen API Documentation" VERBATIM
    WORKING_DIRECTORY "${XTL_SOURCE_DIR}"
  )
endif()

add_subdirectory(examples)

if(GTEST_FOUND)
  add_subdirectory(tests)
endif()


