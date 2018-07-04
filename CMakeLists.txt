### cmake config

cmake_minimum_required(VERSION 3.0)

set(CMAKE_CXX_STANDARD 11)

if(POLICY CMP0048)
  cmake_policy(SET CMP0048 NEW)
endif()
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
include(CheckCXXSourceCompiles)
include(FeatureSummary)
include(ExternalProject)

get_directory_property(hasParent PARENT_DIRECTORY)
set(XTL_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(XTL_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR})

if(hasParent)
  set(XTL_SOURCE_DIR ${XTL_SOURCE_DIR} PARENT_SCOPE)
  set(XTL_BINARY_DIR ${XTL_BINARY_DIR} PARENT_SCOPE)
endif()

option(XTD_LOG_TARGET_SYSLOG "write log output to syslog file" FALSE)
option(XTD_LOG_TARGET_WINDBG "write log output to attached debugger" FALSE)
option(XTD_LOG_TARGET_CSV "write log output to CSV file" FALSE)
option(XTD_LOG_TARGET_COUT "write log output to std::cout" FALSE)
option(XTD_LOG_TARGET_XML "write log output to XML file" FALSE)
option(XTD_VERBOSE_BUILD "print detailed compile messages")
option(XTD_COLOR_MESSAGES "print messages in color" FALSE)
option(XTD_BUILD_EXAMPLES "Build examples" FALSE)
option(XTD_BUILD_TESTS "Build tests" FALSE)
option(XTD_BUILD_DOCS "Build documentation" FALSE)
option(XTD_BUILD_UTILS "Build utilities" FALSE)
option(XTD_BUILD_NLP "Build NLP" FALSE)
option(XTD_BUILD_SQLITE "Build SQLite" FALSE)

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEBUG")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} -DDEBUG")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -DNDEBUG")
set(CMAKE_CXX_FLAGS_MINSIZEREL "${CMAKE_CXX_FLAGS_MINSIZEREL} -DNDEBUG")

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  set(XTD_CONFIG XTD_CONFIG_DEBUG)
else()
  set(XTD_CONFIG XTD_CONFIG_RELEASE)
endif()


### detect OS
if(APPLE)
  set(XTD_OS XTD_OS_OSX)
elseif(MINGW)
  set(XTD_OS XTD_OS_WINDOWS)
elseif(WIN32)
  set(XTD_OS XTD_OS_WINDOWS)
elseif(CYGWIN)
  set(XTD_OS XTD_OS_UNIX)
elseif(UNIX)
  set(XTD_OS XTD_OS_UNIX)
else()
  message(FATAL_ERROR "Platform not supported")
endif()


### detect compiler
if(MSVC)
  set(XTD_COMPILER XTD_COMPILER_MSVC)
elseif(BORLAND)
  set(XTD_COMPILER XTD_COMPILER_BCC)
elseif(WATCOM)
  set(XTD_COMPILER XTD_COMPILER_WCL)
elseif(MINGW)
  set(XTD_COMPILER XTD_COMPILER_MINGW)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  set(XTD_COMPILER XTD_COMPILER_GCC)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  set(XTD_COMPILER XTD_COMPILER_CLANG)
else()
  message(FATAL_ERROR "Compiler not supported : ${CMAKE_CXX_COMPILER_ID}")
endif()


### character encoding
set(XTD_CHARSET "XTD_CHARSET_MBCS" CACHE STRING "default character set")
set_property(CACHE XTD_CHARSET PROPERTY STRINGS XTD_CHARSET_MBCS XTD_CHARSET_UNICODE)

if(XTD_CHARSET STREQUAL "XTD_CHARSET_MBCS")
  add_definitions(-DMBCS)
else()
  add_definitions(-DUNICODE -D_UNICODE)
endif()

set(ASSET_DIR ${CMAKE_CURRENT_SOURCE_DIR}/assets)

#find some libraries
check_include_file_cxx(codecvt XTD_HAS_CODECVT)
if(NOT XTD_HAS_CODECVT)
  set(XTD_HAS_CODECVT FALSE)
endif()

check_include_file_cxx("experimental/codecvt" XTD_HAS_EXP_CODECVT)
if(NOT XTD_HAS_EXP_CODECVT)
  set(XTD_HAS_EXP_CODECVT FALSE)
endif()


set(XTD_HAS_FILESYSTEM FALSE)
set(XTD_HAS_EXP_FILESYSTEM FALSE)

check_cxx_source_compiles(
  "#include <filesystem>\n int main(){ std::filesystem::path oPath; return 0;}"
  XTD_HAS_FILESYSTEM
)
if(NOT XTD_HAS_FILESYSTEM)
  check_cxx_source_compiles(
    "#include <experimental/filesystem>\n int main(){ std::experimental::filesystem::path oPath; return 0;}"
    XTD_HAS_EXP_FILESYSTEM
  )
endif()

#linux specifics
if(NOT XTD_OS STREQUAL "XTD_OS_UNIX")
  set(XTD_HAS_UUID_LIB FALSE)
  set(XTD_HAS_UUID_H FALSE)
  set(XTD_HAS_UUID FALSE)
  set(XTD_HAS_ICONV_LIB FALSE)
  set(XTD_HAS_ICONV_H FALSE)
  set(XTD_HAS_ICONV FALSE)
else()

  check_library_exists(uuid uuid_generate "" XTD_HAS_UUID_LIB)
  check_include_file_cxx("uuid.h" XTD_HAS_UUID_H)
  check_include_file_cxx("uuid/uuid.h" XTD_HAS_UUID_UUID_H)
  check_include_file_cxx("linux/uuid.h" XTD_HAS_LINUX_UUID_H)
  if(XTD_HAS_UUID_UUID_H)
    set(XTD_HAS_UUID_H TRUE)
    set(XTD_UUID_H "uuid/uuid.h")
  elseif(XTD_HAS_LINUX_UUID_H)
    set(XTD_HAS_UUID_H TRUE)
    set(XTD_UUID_H "linux/uuid.h")
  endif()

  if(XTD_HAS_UUID_LIB AND XTD_HAS_UUID_H)
    set(XTD_HAS_UUID TRUE)
    link_libraries(uuid)
  else()
    set(XTD_HAS_UUID_H FALSE)
  endif()


  find_library(XTD_LIB_ICONV_PATH iconv)
  if(XTD_LIB_ICONV_PATH)
    set(XTD_HAS_ICONV_LIB TRUE)
  endif()
  check_include_file_cxx("iconv.h" XTD_HAS_ICONV_H)
  if(XTD_HAS_ICONV_LIB AND XTD_HAS_ICONV_H)
    set(XTD_HAS_ICONV TRUE)
    link_libraries(iconv)
  else()
    set(XTD_HAS_ICONV FALSE)
  endif()

  link_libraries(dl pthread stdc++)  

endif()


#windows specifics
if(NOT XTD_OS STREQUAL "XTD_OS_WINDOWS")
  set(XTD_HAS_DBGHLP_H FALSE)
  set(XTD_HAS_DBGHELP_LIB FALSE)
  set(XTD_USE_DBGHELP FALSE)
else()

  check_include_file_cxx(dbghelp.h XTD_HAS_DBGHLP_H CACHE BOOLEAN FALSE)
  if(XTD_HAS_DBGHLP_H AND XTD_HAS_DBGHELP_LIB)
    option(XTD_USE_DBGHELP "use dbghelp library" TRUE)
  else()
    set(XTD_USE_DBGHELP FALSE)
  endif()

  if(XTD_USE_DBGHELP)
    link_libraries(dbghelp)
  endif()

  link_libraries(rpcrt4 psapi ws2_32)
  
endif()


#gcc specfics
if(XTD_COMPILER STREQUAL "XTD_COMPILER_GCC" OR XTD_COMPILER STREQUAL "XTD_COMPILER_CLANG" OR XTD_COMPILER STREQUAL "XTD_COMPILER_MINGW")
  set(EVT_TRACE_SRC src/event_trace/evt_gcc.cpp)
  add_compile_options(-std=c++11 -Wall -Wextra -Weffc++)

  if ("Debug" STREQUAL "${CMAKE_BUILD_TYPE}" OR "RelWithDebInfo" STREQUAL "${CMAKE_BUILD_TYPE}")
    add_compile_options(-ggdb3 -fvar-tracking-assignments -DSF_VISIBILITY -fvisibility=hidden -fno-strict-aliasing)
  endif()

  if(XTD_HAS_FILESYSTEM OR XTD_HAS_EXP_FILESYSTEM)
    link_libraries(stdc++fs)
  endif()

endif()


### MSVC specifics
if( XTD_COMPILER STREQUAL "XTD_COMPILER_MSVC")
  if(CMAKE_CL_64)
    enable_language(ASM_MASM)
    set(EVT_TRACE_SRC src/event_trace/evt_vc64.asm)
    set_property(SOURCE src/event_trace/evt_vc64.asm PROPERTY LANGUAGE ASM_MASM)
  else()
    set(EVT_TRACE_SRC src/event_trace/evt_vc32.cpp)
  endif()
endif()


### MINGW specifics
if(XTD_COMPILER STREQUAL "XTD_COMPILER_MINGW")
  add_compile_options(-std=c++11 -Wall -Wextra -Weffc++ -enable-libstdcxx-filesystem-ts)
  link_libraries(pthread stdc++)
  if(XTD_HAS_FILESYSTEM OR XTD_HAS_EXP_FILESYSTEM)
    link_libraries(stdc++fs)
  endif()
endif()


### MINGW and MSVC specifics
if(NOT XTD_COMPILER STREQUAL "XTD_COMPILER_MSVC" AND NOT XTD_COMPILER STREQUAL "XTD_COMPILER_MINGW")
  set(XTD_STATIC_CRT FALSE)
else()
  option(XTD_STATIC_CRT "Use static CRT" TRUE)
  if(XTD_STATIC_CRT)
    set(USE_STATIC_RT TRUE)
  endif()  
endif()


### set default character set and conversion
set(XTD_STR_CONVERT XTD_STR_CONVERT_GENERIC CACHE STRING "default string conversion backend")
set_property(CACHE XTD_STR_CONVERT PROPERTY STRINGS XTD_STR_CONVERT_GENERIC XTD_STR_CONVERT_CODECVT XTD_STR_CONVERT_ICONV)


configure_file(${CMAKE_CURRENT_SOURCE_DIR}/include/xtd/xtd.hpp.in ${CMAKE_CURRENT_SOURCE_DIR}/include/xtd/xtd.hpp @ONLY)

include_directories(include)

add_library(XTL STATIC 
  ${EVT_TRACE_SRC}
  ${SQLITE_SRC}
  src/xtl.cpp
  src/event_trace/event_trace.cpp
  include/xtd/com/com.hpp
  include/xtd/com/com_class_factory.hpp
  include/xtd/com/com_data_object.hpp
  include/xtd/com/com_dll.hpp
  include/xtd/com/com_exception.hpp
  include/xtd/com/com_object.hpp
  include/xtd/com/com_initializer.hpp
  include/xtd/com/com_pointer.hpp
  include/xtd/concurrent/concurrent.hpp
  include/xtd/concurrent/hash_map.hpp
  include/xtd/concurrent/queue.hpp
  include/xtd/concurrent/recursive_spin_lock.hpp
  include/xtd/concurrent/rw_lock.hpp
  include/xtd/concurrent/spin_lock.hpp
  include/xtd/concurrent/stack.hpp
  include/xtd/nlp/moby.hpp
  include/xtd/nlp/wordnet.hpp
  include/xtd/windows/debug_help.hpp
  include/xtd/callback.hpp
  include/xtd/debug.hpp
  include/xtd/dynamic_library.hpp
  include/xtd/dynamic_object.hpp
  include/xtd/event_trace.hpp
  include/xtd/exception.hpp
  include/xtd/executable.hpp
  include/xtd/filesystem.hpp
  include/xtd/lru_cache.hpp
  include/xtd/memory.hpp
  include/xtd/mapped_file.hpp
  include/xtd/mapped_vector.hpp
  include/xtd/btree.hpp
  include/xtd/log.hpp
  include/xtd/meta.hpp
  include/xtd/parse.hpp
  include/xtd/process.hpp
  include/xtd/rpc.hpp
  include/xtd/shared_mem_obj.hpp
  include/xtd/socket.hpp
  include/xtd/source_location.hpp
  include/xtd/sqlite.hpp
  include/xtd/string.hpp
  include/xtd/tuple.hpp
  include/xtd/unique_id.hpp
  include/xtd/var.hpp
  include/xtd/wrapped_type.hpp
  include/xtd/xtd.hpp
  include/xtd/xtd.hpp.in
  )

link_libraries(XTL)

install(TARGETS XTL
  RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin
  ARCHIVE DESTINATION ${CMAKE_INSTALL_PREFIX}/lib
)

install(DIRECTORY include DESTINATION ${CMAKE_INSTALL_PREFIX} FILES_MATCHING PATTERN "*.hpp")

if(XTD_BUILD_TESTS)
  add_subdirectory(tests)
endif()

if(XTD_BUILD_EXAMPLES)
  add_subdirectory(examples)
endif()

if(XTD_BUILD_DOCS AND DOXYGEN_FOUND)
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/docs/Doxyfile.in ${CMAKE_CURRENT_SOURCE_DIR}/docs/Doxyfile)
  add_custom_target(
    docs COMMAND "${DOXYGEN_EXECUTABLE}" Doxyfile
    COMMENT "Generating Doxygen API Documentation" VERBATIM
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/docs"
  )
endif()

