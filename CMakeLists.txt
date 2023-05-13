cmake_minimum_required(VERSION 3.22)
project(TicketSystem)

set(CMAKE_CXX_STANDARD 20)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --coverage")

include_directories(src utility utility/stlite)


option(USE_STL "is use stl as bottom level data structure" ON)
if (USE_STL)
    add_definitions(-DUSE_STL)
    add_definitions(-DBPT_USE_STL)
endif ()

file(GLOB_RECURSE sources RELATIVE ${PROJECT_SOURCE_DIR} src/**.cpp)

add_executable(main ${sources})
set(test_prefix "${PROJECT_SOURCE_DIR}/data")
file(GLOB tests RELATIVE "${test_prefix}" data/*)

message(STATUS ${tests})

foreach (test-name IN ITEMS ${tests})
    if (NOT IS_DIRECTORY "${test_prefix}/${test-name}")
        continue()
    endif ()
    message(STATUS "${test-name}")
    add_test(NAME "${test-name}"
            COMMAND bash ../run-test.sh "${test_prefix}" "${test-name}" "$<TARGET_FILE:main>"
            WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/run
            )
endforeach ()

enable_testing()
