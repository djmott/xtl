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

message(STATUS "Host system is ${CMAKE_HOST_SYSTEM_NAME}")

enable_language(CXX)

set(default_build_type "Release")
if(EXISTS "${CMAKE_SOURCE_DIR}/.git")
  set(default_build_type "Debug")
endif()
 
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE "${default_build_type}" CACHE STRING "Choose the type of build." FORCE)
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()
message(STATUS "Build type is ${CMAKE_BUILD_TYPE}")

project(XTL)

include(CheckIncludeFile)
include(CheckIncludeFileCXX)
include(ExternalProject)

include_directories(include)

option(XTD_VERBOSE_BUILD "Verbose builds" TRUE)
set(XTD_HAS_CODECVT 0)
set(XTD_HAS_EXP_CODECVT 0)
set(XTD_HAS_ICONV 0)
set(XTD_HAS_FILESYSTEM 1)

option(XTD_BUILD_EXAMPLES "Build examples" FALSE)
if(XTD_BUILD_EXAMPLES)
  add_subdirectory(examples)  
endif()

option(XTD_BUILD_TESTS "Build tests" FALSE)
if(XTD_BUILD_TESTS)
  add_subdirectory(tests)
endif()

set(XTD_OS "XTD_OS_BAREMETAL")
if(ANDROID)
  set(XTD_OS "XTD_OS_ANDROID")
elseif(MINGW)
  set(XTD_OS "XTD_OS_MINGW")
elseif(CYGWIN)
  set(XTD_OS "XTD_OS_CYGWIN")
elseif(WIN32)
  set(XTD_OS "XTD_OS_WINDOWS")
elseif(UNIX)
  set(XTD_OS "XTD_OS_UNIX")
elseif(APPLE)
  set(XTD_OS "XTD_OS_IOS")
else()
  message(FATAL_ERROR "Target OS not supported")
endif()

if("MSYS" STREQUAL "${CMAKE_HOST_SYSTEM_NAME}" OR "CYGWIN" STREQUAL "${CMAKE_HOST_SYSTEM_NAME}")
  set(XTD_OS "XTD_OS_CYGWIN")
endif()


set(XTD_COMPILER "XTD_COMPILER_UNKNOWN")
if("MSVC" STREQUAL "${CMAKE_CXX_COMPILER_ID}")
  set(XTD_COMPILER "XTD_COMPILER_MSVC")
elseif("GNU" STREQUAL "${CMAKE_CXX_COMPILER_ID}")
  set(XTD_COMPILER "XTD_COMPILER_GCC")
elseif("Clang" STREQUAL "${CMAKE_CXX_COMPILER_ID}")
  set(XTD_COMPILER "XTD_COMPILER_CLANG")
elseif("Intel" STREQUAL "${CMAKE_CXX_COMPILER_ID}")
  set(XTD_COMPILER "XTD_COMPILER_INTEL")
elseif("OpenWatcom" STREQUAL "${CMAKE_CXX_COMPILER_ID}")
  set(XTD_COMPILER "XTD_COMPILER_WATCOM")
elseif("Embarcadero" STREQUAL "${CMAKE_CXX_COMPILER_ID}")
  set(XTD_COMPILER "XTD_COMPILER_BORLAND")
elseif("Borland" STREQUAL "${CMAKE_CXX_COMPILER_ID}")
  set(XTD_COMPILER "XTD_COMPILER_BORLAND")
elseif("ARMCC" STREQUAL "${CMAKE_CXX_COMPILER_ID}")
  set(XTD_COMPILER "XTD_COMPILER_ARM")
else()
  message(FATAL_ERROR "Compiler not supported")
endif()


set(XTD_CHARSET "XTD_CHARSET_UNICODE" CACHE STRING "Default character set." FORCE)
set_property(CACHE XTD_CHARSET PROPERTY STRINGS "XTD_CHARSET_UNICODE" "XTD_CHARSET_MBCS")

set(XTD_CONFIG "XTD_CONFIG_DEBUG")
if("Release" STREQUAL "${CMAKE_BUILD_TYPE}")
  set(XTD_CONFIG "XTD_CONFIG_RELEASE")
elseif("MinSizeRel" STREQUAL "${CMAKE_BUILD_TYPE}")
  set(XTD_CONFIG "XTD_CONFIG_MINSIZE")
elseif("RelWithDebInfo" STREQUAL "${CMAKE_BUILD_TYPE}")
  set(XTD_CONFIG "XTD_CONFIG_RELDEBUG")
endif()

option(XTD_LOG_TARGET_SYSLOG "Use syslog debug target")
option(XTD_LOG_TARGET_WINDBG "Use windbg debug target")
option(XTD_LOG_TARGET_CSV "Use csv debug target")
option(XTD_LOG_TARGET_COUT "Use cout debug target")
option(XTD_LOG_TARGET_XML "Use xml debug target")


set(XTD_HAS_UUID FALSE)
if(UNIX)
  CHECK_INCLUDE_FILE_CXX("uuid.h" UUID_H_EXISTS)
  if(UUID_H_EXISTS)
    set(XTD_UUID_H "uuid.h")
    set(XTD_HAS_UUID TRUE)
  endif()
  CHECK_INCLUDE_FILE_CXX("uuid/uuid.h" UUID_UUID_H_EXISTS)
  if(UUID_UUID_H_EXISTS)
    set(XTD_UUID_H "uuid/uuid.h")
    set(XTD_HAS_UUID TRUE)
  endif()
endif()

option(XTD_COLOR_MESSAGES "Use colored console" FALSE)

set(XTD_COM_HEADERS
  include/xtd/com/com.hpp
  include/xtd/com/com_class_factory.hpp
  include/xtd/com/com_data_object.hpp
  include/xtd/com/com_dll.hpp
  include/xtd/com/com_exception.hpp
  include/xtd/com/com_initializer.hpp
  include/xtd/com/com_object.hpp
  include/xtd/com/com_pointer.hpp
)

set(XTD_CONCURRENT_HEADERS
  include/xtd/concurrent/concurrent.hpp
  include/xtd/concurrent/hash_map.hpp
  include/xtd/concurrent/queue.hpp
  include/xtd/concurrent/recursive_spin_lock.hpp
  include/xtd/concurrent/rw_lock.hpp
  include/xtd/concurrent/spin_lock.hpp
  include/xtd/concurrent/stack.hpp
)

set(XTD_GRAMMARS
  include/xtd/grammars/iso32000-1.hpp
  include/xtd/grammars/RFC1035.hpp
  include/xtd/grammars/RFC1123.hpp
  include/xtd/grammars/RFC1950.hpp
  include/xtd/grammars/RFC1951.hpp
  include/xtd/grammars/RFC1952.hpp
  include/xtd/grammars/rfc2229.hpp
  include/xtd/grammars/RFC2234.hpp
  include/xtd/grammars/RFC2460.hpp
  include/xtd/grammars/RFC2616.hpp
  include/xtd/grammars/RFC3066.hpp
  include/xtd/grammars/RFC3513.hpp
  include/xtd/grammars/RFC3986.hpp
  include/xtd/grammars/RFC4647.hpp
  include/xtd/grammars/RFC5234.hpp
  include/xtd/grammars/RFC5322.hpp
  include/xtd/grammars/RFC5646.hpp
  include/xtd/grammars/RFC7230.hpp
  include/xtd/grammars/RFC7231.hpp
  include/xtd/grammars/RFC7232.hpp
  include/xtd/grammars/RFC7233.hpp
  include/xtd/grammars/RFC7234.hpp
  include/xtd/grammars/RFC7235.hpp
  include/xtd/grammars/RFC822.hpp
  include/xtd/grammars/RFC850.hpp
  include/xtd/grammars/XMLParse.hpp
)

set(XTD_NLP_HEADERS
  include/xtd/nlp/case_converter.hpp
  include/xtd/nlp/document.hpp
  include/xtd/nlp/doc_filter/acronym.hpp
  include/xtd/nlp/doc_filter/doc_filter.hpp
  include/xtd/nlp/doc_filter/hyphenated_word.hpp
  include/xtd/nlp/moby.hpp
  include/xtd/nlp/ngram_stemmer.hpp
  include/xtd/nlp/nlp.hpp
  include/xtd/nlp/porter_stemmer.hpp
  include/xtd/nlp/scowl_pos_tagger.hpp
  include/xtd/nlp/simple_PBD.hpp
  include/xtd/nlp/simple_SBD.hpp
  include/xtd/nlp/simple_tokenizer.hpp
  include/xtd/nlp/statistical_stemmer.hpp
  include/xtd/nlp/text_processor.hpp
  include/xtd/nlp/wordnet.hpp
  include/xtd/nlp/wordnet_old.hpp
)

set(XTD_WINDOWS_HEADERS
  include/xtd/windows/debug_help.hpp
  include/xtd/windows/image_help.hpp
)

set(XTD_HEADERS
  include/xtd/btree.hpp
  include/xtd/callback.hpp
  include/xtd/debug.hpp
  include/xtd/decorated_object.hpp
  include/xtd/dynamic_library.hpp
  include/xtd/dynamic_object.hpp
  include/xtd/event_trace.hpp
  include/xtd/exception.hpp
  include/xtd/executable.hpp
  include/xtd/filesystem.hpp
  include/xtd/log.hpp
  include/xtd/lru_cache.hpp
  include/xtd/mapped_file.hpp
  include/xtd/mapped_vector.hpp
  include/xtd/memory.hpp
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
  include/xtd/type_info.hpp
  include/xtd/unique_id.hpp
  include/xtd/var.hpp
  include/xtd/wrapped_type.hpp
  include/xtd/xtd.hpp
)

set(XTD_TEST_HEADERS
  tests/mocks/rpc.hpp
  tests/test_btree.hpp
  tests/test_callback.hpp
  tests/test_com.hpp
  tests/test_concurrent_stack.hpp
  tests/test_debug_help.hpp
  tests/test_dynamic_library.hpp
  tests/test_event_trace.hpp
  tests/test_exception.hpp
  tests/test_executable.hpp
  tests/test_hash_map.hpp
  tests/test_logging.hpp
  tests/test_lru_cache.hpp
  tests/test_mapped_file.hpp
  tests/test_mapped_vector.hpp
  tests/test_meta.hpp
  tests/test_parse.hpp
  tests/test_path.hpp
  tests/test_process.hpp
  tests/test_recursive_spin_lock.hpp
  tests/test_rpc.hpp
  tests/test_rw_lock.hpp
  tests/test_shared_mem_obj.hpp
  tests/test_socket.hpp
  tests/test_source_location.hpp
  tests/test_spin_lock.hpp
  tests/test_stack.hpp
  tests/test_string.hpp
  tests/test_unique_id.hpp
  tests/test_var.hpp
)

configure_file(include/xtd/xtd.hpp.in include/xtd/xtd.hpp @ONLY)

add_library(xtl STATIC src/xtl.cpp)

target_include_directories(xtl PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include ${CMAKE_CURRENT_BINARY_DIR}/include)

install(FILES ${XTD_COM_HEADERS}          DESTINATION include/xtd/com)
install(FILES ${XTD_COM_HEADERS}          DESTINATION include/xtd/com)
install(FILES ${XTD_CONCURRENT_HEADERS}   DESTINATION include/xtd/concurrent)
install(FILES ${XTD_GRAMMARS}             DESTINATION include/xtd/grammars)
install(FILES ${XTD_NLP_HEADERS}          DESTINATION include/xtd/nlp)
install(FILES ${XTD_WINDOWS_HEADERS}      DESTINATION include/xtd/windows)
install(FILES ${XTD_HEADERS} ${CMAKE_CURRENT_BINARY_DIR}/include/xtd/xtd.hpp DESTINATION include/xtd)
