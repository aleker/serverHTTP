cmake_minimum_required(VERSION 3.6)
project(serwerHTTP)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -pthread")

add_executable(serwerHTTP
        serwerHTTP.cpp
        Record.cpp
        StreamRecord.cpp
        BeginRecord.cpp
        ConnectionManager.cpp
        Parser.cpp
        HTTPManager.cpp
        FCGIManager.cpp)

add_executable(fcgi fcgi.cpp)

# adding flags only for fcgi:
# set_source_files_properties(fcgi.cpp PROPERTIES COMPILE_FLAGS -lfcgi++ -lfcgi)