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

FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/52eb8108c5bdec04579160ae17225d66034bd723.zip
        GIT_PROGRESS TRUE
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(googletest)
FetchContent_MakeAvailable(raylib)

enable_testing()

set(CORE_SOURCES
        src/Window.cpp
        src/Window.h
        src/raylib_logger.h
        src/Graph.cpp
        src/Graph.h
        src/PathFinding.cpp
        src/PathFinding.h
)

# Main executable that runs the application

add_executable(
        priority_routing 
        src/main.cpp
        ${CORE_SOURCES}
)
target_link_libraries(
        priority_routing
        PRIVATE
        raylib
        spdlog::spdlog
)

# Test executable for testing

add_executable(
        priority_routing_tests
        tests/example_test.cpp
        ${CORE_SOURCES}
)

target_link_libraries(
        priority_routing_tests
        PRIVATE
        raylib
        spdlog::spdlog
        GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(priority_routing_tests)
