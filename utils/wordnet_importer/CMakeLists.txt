cmake_minimum_required(VERSION 2.8.12)
project(wordnet_importer)

find_package(Threads)

enable_language(CXX)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES main.cpp
    wordnet.hpp
    )
add_executable(wordnet_importer ${SOURCE_FILES})
include_directories("../../include")

if(CMAKE_SYSTEM_NAME MATCHES "Darwin")
  link_libraries(pthread)
elseif(CMAKE_SYSTEM_NAME MATCHES "MSYS")
  link_libraries(pthread)
elseif(CMAKE_SYSTEM_NAME MATCHES "Linux")
  link_libraries(pthread)
elseif(CMAKE_SYSTEM_NAME MATCHES "CYGWIN")
  link_libraries(pthread)
endif()
