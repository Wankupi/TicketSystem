cmake_minimum_required(VERSION 3.15)
project(TicketSystem)

set(CMAKE_CXX_STANDARD 17)

include_directories(src utility utility/stlite)

set(sources src/main.cpp
        src/terminal/TicketTerminal.cpp
        src/user/user.cpp
        )

add_executable(main ${sources})
