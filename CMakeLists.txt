cmake_minimum_required(VERSION 4.1)
project(priority_routing)

set(CMAKE_CXX_STANDARD 26)

include(FetchContent)

set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(BUILD_GAMES    OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
        raylib
        GIT_REPOSITORY https://github.com/raysan5/raylib.git
        GIT_TAG 97023def48c4be1e27bbaab70242521dd0d829df
        GIT_PROGRESS TRUE
)

FetchContent_MakeAvailable(raylib)

add_executable(priority_routing main.cpp
        Window.cpp
        Window.h)
target_link_libraries(
        priority_routing
        raylib
)