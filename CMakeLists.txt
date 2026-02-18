cmake_minimum_required(VERSION 4.1)
project(priority_routing)

set(CMAKE_CXX_STANDARD 26)

include(FetchContent)

set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(BUILD_GAMES    OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
        raylib
        GIT_REPOSITORY https://github.com/raysan5/raylib.git
        GIT_TAG 5.5
        GIT_PROGRESS TRUE
)

FetchContent_Declare(
        tinyxml2
        GIT_REPOSITORY https://github.com/leethomason/tinyxml2.git
        GIT_TAG 11.0.0
        GIT_PROGRESS TRUE
)

FetchContent_MakeAvailable(raylib)
FetchContent_MakeAvailable(tinyxml2)

add_executable(priority_routing main.cpp
        Window.cpp
        Window.h
        httplib.h
        httplib.cpp
        DataFetcher.cpp)
        
target_link_libraries(
        priority_routing
        raylib
        tinyxml2
)