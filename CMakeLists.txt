cmake_minimum_required(VERSION 3.6)
project(serwerHTTP)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES main.cpp example.cpp)
add_executable(serwerHTTP ${SOURCE_FILES})