cmake_minimum_required(VERSION 3.22)
project(TicketSystem)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --coverage")

include_directories(src utility utility/stlite)

#add_definitions(-DUSE_STL)

file(GLOB_RECURSE sources RELATIVE ${PROJECT_SOURCE_DIR} src/**.cpp)

add_executable(main ${sources})
