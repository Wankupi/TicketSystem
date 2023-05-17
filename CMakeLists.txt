cmake_minimum_required(VERSION 3.22)
project(TicketSystem)

set(CMAKE_CXX_STANDARD 20)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2")

include_directories(src utility utility/stlite)

option(USE_STL "is use stl as bottom level data structure" ON)
if (USE_STL)
    add_definitions(-DUSE_STL)
    add_definitions(-DBPT_USE_STL)
endif ()

file(GLOB_RECURSE sources RELATIVE ${PROJECT_SOURCE_DIR} src/*/*.cpp)

add_executable(code ${sources} src/main.cpp)
