### cmake config

cmake_minimum_required(VERSION 2.8.12)

set(CMAKE_LEGACY_CYGWIN_WIN32 0)
set(CMAKE_CXX_STANDARD 11)

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
if(hasParent)
  set(XTL_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR} PARENT_SCOPE)
  set(XTL_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR} PARENT_SCOPE)
endif()

set(ASSET_DIR ${CMAKE_CURRENT_SOURCE_DIR}/assets)

#[[
check_library_exists(uuid uuid_generate "" XTD_HAS_LIBUUID)


check_cxx_source_compiles("
  #include <codecvt>
  int main(){return 0;}
  " XTD_HAS_CODECVT)

check_cxx_source_compiles("
  #include <experimental/codecvt>
  int main(){return 0;}
  " XTD_HAS_EXP_CODECVT)

check_cxx_source_compiles("
  #include <filesystem>
  int main(){ return 0; }
  " XTD_HAS_FILESYSTEM)

check_cxx_source_compiles("
  #include <experimental/filesystem>
  int main(){ return 0; }
  " XTD_HAS_EXP_FILESYSTEM)


check_cxx_source_compiles("
  #include <iconv.h>
  int main(){ return 0; }
  " XTD_HAS_ICONV_H)

check_library_exists(iconv iconv_open "" XTD_HAS_ICONV_LIB)

check_include_file_cxx(dbghelp.h XTD_HAS_DBGHLP_H CACHE BOOLEAN FALSE)
check_library_exists(dbghelp.lib SymInitialize "" XTD_HAS_DBGHELP_LIB)

if(XTD_HAS_ICONV_LIB AND XTD_HAS_ICONV_H)
  set(XTD_HAS_ICONV 1)
else()
  set(XTD_HAS_ICONV 0)
endif()

if(XTD_HAS_DBGHLP_H OR XTD_HAS_DBGHELP_LIB)
  option(XTD_USE_DBGHELP "use dbghelp library" YES)
else()
  option(XTD_USE_DBGHELP "use dbghelp library" NO)
endif()
]]

option(XTD_VERBOSE_BUILD "print detailed compile messages")
option(XTD_COLOR_MESSAGES "use color escape sequences in console compiler output" NO)
option(XTD_LOG_TARGET_CSV "write log output to CSV file" NO)
option(XTD_LOG_TARGET_COUT "write log output to std::cout" NO)
option(XTD_LOG_TARGET_XML "write log output to XML file" NO)
option(XTD_LOG_TARGET_SYSLOG "write log output to syslog file" NO)
option(XTD_LOG_TARGET_WINDBG "write log output to attached debugger" NO)
option(BUILD_EXAMPLES "Build examples" NO)
option(BUILD_TESTS "Build tests" NO)
option(BUILD_DOCS "Build documentation" NO)
option(BUILD_UTILS "Build utilities" NO)
option(BUILD_NLP "Build NLP" NO)
option(BUILD_SQLITE "Build SQLite" NO)

#option(XTD_LINK_TO_FS_TS "link to the c++ filesystem TS" NO)

### detect OS
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

### detect compiler
if(MSVC)
  set(XTD_COMPILER XTD_COMPILER_MSVC)
elseif(BORLAND)
  set(XTD_COMPILER XTD_COMPILER_BCC)
elseif(WATCOM)
  set(XTD_COMPILER XTD_COMPILER_WCL)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  set(XTD_COMPILER XTD_COMPILER_GCC)
  set(XTD_HAS_LIBUUID on)
else()
  message(FATAL_ERROR "Compiler not supported : ${CMAKE_CXX_COMPILER_ID}")
endif()


set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEBUG")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} -DDEBUG")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -DNDEBUG")
set(CMAKE_CXX_FLAGS_MINSIZEREL "${CMAKE_CXX_FLAGS_MINSIZEREL} -DNDEBUG")


### set event trace compiler backend
if(XTD_COMPILER MATCHES "XTD_COMPILER_GCC")
  set(EVT_TRACE_SRC src/event_trace/evt_gcc.cpp)
elseif(XTD_COMPILER MATCHES "XTD_COMPILER_MSVC")
  if(CMAKE_CL_64)
    enable_language(ASM_MASM)
    set(EVT_TRACE_SRC src/event_trace/evt_vc64.asm)
    set_property(SOURCE src/event_trace/evt_vc64.asm PROPERTY LANGUAGE ASM_MASM)
  else()
    set(EVT_TRACE_SRC src/event_trace/evt_vc32.cpp)
  endif()
else()
  message(FATAL_ERROR "event trace not supported for compiler")
endif()

if(BUILD_NLP)
	if(NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/assets/sqlite-autoconf-3140100.tar.gz)
	  message("Downloading sqlite")
	  file(DOWNLOAD https://www.sqlite.org/2016/sqlite-autoconf-3140100.tar.gz ${CMAKE_CURRENT_SOURCE_DIR}/assets/sqlite-autoconf-3140100.tar.gz)
	  execute_process(
		COMMAND ${CMAKE_COMMAND} -E tar xzf ${CMAKE_CURRENT_SOURCE_DIR}/assets/sqlite-autoconf-3140100.tar.gz
		WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/assets/
	  )
	endif()
endif()

if(BUILD_SQLITE)
	set(SQLITE_SRC ${CMAKE_CURRENT_SOURCE_DIR}/assets/sqlite-autoconf-3140100/sqlite3.c)
endif()

add_library(XTL STATIC ${EVT_TRACE_SRC}
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
  )

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

if(BUILD_SQLITE)
	include_directories(${CMAKE_CURRENT_SOURCE_DIR}/assets/sqlite-autoconf-3140100)
endif()

if(XTD_COMPILER MATCHES "XTD_COMPILER_GCC")
  target_compile_options(XTL PUBLIC -std=c++11 -Wall -Wextra)
endif()

link_libraries(XTL)

if(XTD_HAS_LIBUUID)
  link_libraries(uuid)
endif()

### set default character set and conversion
set(XTD_STR_CONVERT XTD_STR_CONVERT_GENERIC CACHE STRING "default string conversion backend")
set_property(CACHE XTD_STR_CONVERT PROPERTY STRINGS XTD_STR_CONVERT_GENERIC XTD_STR_CONVERT_CODECVT XTD_STR_CONVERT_ICONV)

if(XTD_HAS_CODECVT OR XTD_HAS_EXP_CODECVT)
elseif(XTD_HAS_ICONV)
  link_libraries(iconv)
endif()

set(XTD_CHARSET "XTD_CHARSET_MBCS" CACHE STRING "default character set")
set_property(CACHE XTD_CHARSET PROPERTY STRINGS XTD_CHARSET_MBCS XTD_CHARSET_UNICODE)

if(XTD_CHARSET STREQUAL "XTD_CHARSET_MBCS")
  add_definitions(-DMBCS)
else()
  add_definitions(-DUNICODE)
endif()

if(XTD_LINK_TO_FS_TS)
  link_libraries(stdc++fs)
endif()

### link libraries
if(XTD_OS MATCHES "XTD_OS_CYGWIN")
  link_libraries(dl pthread stdc++)
elseif(XTD_OS MATCHES "XTD_OS_MINGW")
  link_libraries(rpcrt4 ws2_32 pthread stdc++ psapi)
elseif(XTD_OS MATCHES "XTD_OS_LINUX")
  link_libraries(dl pthread stdc++)
  if(XTD_LINK_TO_FS_TS)
    link_libraries(stdc++fs)
  endif()
elseif(XTD_OS MATCHES "XTD_OS_WINDOWS")
  link_libraries(rpcrt4 psapi)
endif()


if(XTD_HAS_DBGHLP_H AND XTD_HAS_DBGHELP_LIB)
  set(XTD_HAS_DBGHELP 1)
  link_libraries(dbghelp)
endif()


# download assets
if(BUILD_NLP)
	if(NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/assets/WordNet-3.0.tar.bz2)
	message("Downloading Wordnet")
	file(DOWNLOAD http://wordnetcode.princeton.edu/3.0/WordNet-3.0.tar.bz2 ${CMAKE_CURRENT_SOURCE_DIR}/assets/WordNet-3.0.tar.bz2)
	execute_process(
	  COMMAND ${CMAKE_COMMAND} -E tar xzf ${CMAKE_CURRENT_SOURCE_DIR}/assets/WordNet-3.0.tar.bz2
	  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/assets/
	)
	endif()

	if(NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/assets/mpos.tar.Z)
	message("Downloading Moby")
	file(DOWNLOAD http://www.dcs.shef.ac.uk/research/ilash/Moby/mpos.tar.Z ${CMAKE_CURRENT_SOURCE_DIR}/assets/mpos.tar.Z)
	execute_process(
	  COMMAND ${CMAKE_COMMAND} -E tar xzf ${CMAKE_CURRENT_SOURCE_DIR}/assets/mpos.tar.Z
	  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/assets/
	)
	endif()
endif()

if(BUILD_TESTS)
  find_package(GTest)
  if(NOT GTEST_FOUND)
    if(MINGW)
      set(gtest_disable_pthreads ON CACHE BOOL "" FORCE)
    endif()
    if(MINGW OR MSVC)
      set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    endif()
    set(BUILD_GTEST ON CACHE BOOL "" FORCE)
    set(BUILD_GMOCK ON CACHE BOOL "" FORCE)
    add_subdirectory(${ASSET_DIR}/gtest)
  endif()
  add_subdirectory(tests)
endif()

if(BUILD_DOCS AND DOXYGEN_FOUND)
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

if(BUILD_UTILS AND BUILD_NLP)
  add_subdirectory(utils/wordnet_importer)
  add_subdirectory(utils/scowl_importer)
endif()
