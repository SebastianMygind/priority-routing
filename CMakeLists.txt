cmake_minimum_required(VERSION 4.1)
project(priority_routing)

set(CMAKE_CXX_STANDARD 26)

include(FetchContent)

set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(BUILD_GAMES    OFF CACHE BOOL "" FORCE)

find_package(spdlog REQUIRED)

FetchContent_Declare(
        raylib
        GIT_REPOSITORY https://github.com/raysan5/raylib.git
        GIT_TAG 5.5
        GIT_PROGRESS TRUE
)

FetchContent_MakeAvailable(raylib)

add_executable(priority_routing src/main.cpp
        src/Window.cpp
        src/Window.h
        src/Node.cpp
        src/Node.h)
target_link_libraries(
        priority_routing
        raylib
        spdlog::spdlog
)