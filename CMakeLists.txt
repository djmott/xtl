### cmake config

cmake_minimum_required(VERSION 3.0.2)

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
set(XTL_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(XTL_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR})

if(hasParent)
  set(XTL_SOURCE_DIR ${XTL_SOURCE_DIR} PARENT_SCOPE)
  set(XTL_BINARY_DIR ${XTL_BINARY_DIR} PARENT_SCOPE)
endif()

option(XTD_LOG_TARGET_SYSLOG "write log output to syslog file" NO)
option(XTD_LOG_TARGET_WINDBG "write log output to attached debugger" NO)
option(XTD_LOG_TARGET_CSV "write log output to CSV file" NO)
option(XTD_LOG_TARGET_COUT "write log output to std::cout" NO)
option(XTD_LOG_TARGET_XML "write log output to XML file" NO)
option(XTD_VERBOSE_BUILD "print detailed compile messages")
option(XTD_COLOR_MESSAGES "print messages in color" NO)
option(BUILD_EXAMPLES "Build examples" YES)
option(BUILD_TESTS "Build tests" YES)
option(BUILD_DOCS "Build documentation" NO)
option(BUILD_UTILS "Build utilities" NO)
option(BUILD_NLP "Build NLP" NO)
option(BUILD_SQLITE "Build SQLite" NO)

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
elseif(UNIX OR CYGWIN)
  set(XTD_OS XTD_OS_UNIX)
elseif(MINGW OR WIN32)
  set(XTD_OS XTD_OS_WINDOWS)
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
else()
  message(FATAL_ERROR "Compiler not supported : ${CMAKE_CXX_COMPILER_ID}")
endif()


### character encoding
set(XTD_CHARSET "XTD_CHARSET_MBCS" CACHE STRING "default character set")
set_property(CACHE XTD_CHARSET PROPERTY STRINGS XTD_CHARSET_MBCS XTD_CHARSET_UNICODE)

if(XTD_CHARSET STREQUAL "XTD_CHARSET_MBCS")
  add_definitions(-DMBCS)
else()
  add_definitions(-DUNICODE)
endif()


set(ASSET_DIR ${CMAKE_CURRENT_SOURCE_DIR}/assets)


check_cxx_source_compiles("
  #include <codecvt>
  int main(){return 0;}
  " XTD_HAS_CODECVT)
  if(NOT XTD_HAS_CODECVT)
    set(XTD_HAS_CODECVT NO)
  endif()

check_cxx_source_compiles("
  #include <experimental/codecvt>
  int main(){return 0;}
  " XTD_HAS_EXP_CODECVT)
  if(NOT XTD_HAS_EXP_CODECVT)
    set(XTD_HAS_EXP_CODECVT NO)
  endif()

check_cxx_source_compiles("
  #include <filesystem>
  int main(){ return 0; }
  " XTD_HAS_FILESYSTEM)
  if(NOT XTD_HAS_FILESYSTEM)
    set(XTD_HAS_FILESYSTEM NO)
  endif()

check_cxx_source_compiles("
  #include <experimental/filesystem>
  int main(){ return 0; }
  " XTD_HAS_EXP_FILESYSTEM)
  if(NOT XTD_HAS_EXP_FILESYSTEM)
    set(XTD_HAS_EXP_FILESYSTEM NO)
  endif()

#linux specifics
if(NOT XTD_OS STREQUAL "XTD_OS_UNIX")
  set(XTD_HAS_UUID_LIB NO)
  set(XTD_HAS_UUID_H NO)
  set(XTD_HAS_UUID NO)
  set(XTD_HAS_ICONV_LIB NO)
  set(XTD_HAS_ICONV_H NO)
  set(XTD_HAS_ICONV NO)
else()
  check_library_exists(uuid uuid_generate "" XTD_HAS_UUID_LIB)

  check_cxx_source_compiles("
    #include <uuid.h>
    int main(){ return 0; }
    " XTD_HAS_UUID_H)
    if(NOT XTD_HAS_UUID_H)
      set(XTD_HAS_UUID_H NO)
    endif()

  if(XTD_HAS_UUID_LIB AND XTD_HAS_UUID_H)
    set(XTD_HAS_UUID YES)
    link_libraries(uuid)
  endif()  

  check_cxx_source_compiles("
    #include <iconv.h>
    int main(){ return 0; }
    " XTD_HAS_ICONV_H)
    if(NOT XTD_HAS_ICONV_H)
      set(XTD_HAS_ICONV_H NO)
    endif()

  check_library_exists(iconv iconv_open "" XTD_HAS_ICONV_LIB)

  if(XTD_HAS_ICONV_LIB AND XTD_HAS_ICONV_H)
    set(XTD_HAS_ICONV YES)
  else()
    set(XTD_HAS_ICONV NO)
  endif()

  link_libraries(dl pthread stdc++)  

endif()


#windows specifics
if(NOT XTD_OS STREQUAL "XTD_OS_WINDOWS")
  set(XTD_HAS_DBGHLP_H NO)
  set(XTD_HAS_DBGHELP_LIB NO)
  set(XTD_USE_DBGHELP NO)
else()

  check_include_file_cxx(dbghelp.h XTD_HAS_DBGHLP_H CACHE BOOLEAN FALSE)
  check_library_exists(dbghelp.lib SymInitialize "" XTD_HAS_DBGHELP_LIB)

  if(XTD_HAS_DBGHLP_H AND XTD_HAS_DBGHELP_LIB)
    option(XTD_USE_DBGHELP "use dbghelp library" YES)
  else()
    set(XTD_USE_DBGHELP NO)
  endif()

  if(XTD_HAS_FILESYSTEM OR XTD_HAS_EXP_FILESYSTEM)
    link_libraries(stdc++fs)
  endif()

  link_libraries(rpcrt4 psapi ws2_32 dbghelp)
  
endif()


#gcc specfics
if(XTD_COMPILER MATCHES "XTD_COMPILER_GCC")
  set(EVT_TRACE_SRC src/event_trace/evt_gcc.cpp)
  add_compile_options(-std=c++11 -Wall -Wextra)
endif()


### MSVC specifics
if(XTD_COMPILER MATCHES "XTD_COMPILER_MSVC")
  if(CMAKE_CL_64)
    enable_language(ASM_MASM)
    set(EVT_TRACE_SRC src/event_trace/evt_vc64.asm)
    set_property(SOURCE src/event_trace/evt_vc64.asm PROPERTY LANGUAGE ASM_MASM)
  else()
    set(EVT_TRACE_SRC src/event_trace/evt_vc32.cpp)
  endif()
endif()

### MINGW specifics
if(XTD_COMPILER MATCHES "XTD_COMPILER_MINGW")
  link_libraries(pthread stdc++)
endif()

#[[
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
else()
  set(SQLITE_SRC "")
endif()
]]

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
  )


  link_libraries(XTL)



#[[
  if(BUILD_SQLITE)
	include_directories(${CMAKE_CURRENT_SOURCE_DIR}/assets/sqlite-autoconf-3140100)
endif()
]]

install(TARGETS XTL
  RUNTIME DESTINATION bin
  ARCHIVE DESTINATION  lib
)
install(DIRECTORY include DESTINATION include)

#[[
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
]]

if(BUILD_TESTS)
  #[[
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
  ]]
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
#[[
if(BUILD_UTILS AND BUILD_NLP)
  add_subdirectory(utils/wordnet_importer)
  add_subdirectory(utils/scowl_importer)
endif()
]]

