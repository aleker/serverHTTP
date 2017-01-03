cmake_minimum_required(VERSION 3.6)
project(serwerHTTP)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -pthread")

add_executable(serwerTCP serwerTCP.cpp)
add_executable(fcgi fcgi.cpp)

# adding flags only for fcgi:
# set_source_files_properties(fcgi.cpp PROPERTIES COMPILE_FLAGS -lfcgi++ -lfcgi)