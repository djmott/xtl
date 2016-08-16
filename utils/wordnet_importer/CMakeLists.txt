cmake_minimum_required(VERSION 3.6)
project(wordnet_importer)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES main.cpp
    wordnet.hpp
    )
add_executable(wordnet_importer ${SOURCE_FILES})
include_directories("../../include")
target_link_libraries(wordnet_importer pthread)