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

if(XTD_OS MATCHES "XTD_OS_CYGWIN")
  link_libraries(pthread)
elseif(XTD_OS MATCHES "XTD_OS_MINGW")
  link_libraries(pthread)
elseif(XTD_OS MATCHES "XTD_OS_LINUX")
  link_libraries(pthread)
endif()
