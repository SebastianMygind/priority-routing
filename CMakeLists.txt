cmake_minimum_required(VERSION 4.1)
project(priority_routing)

set(CMAKE_CXX_STANDARD 26)

include(FetchContent)

set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(BUILD_GAMES    OFF CACHE BOOL "" FORCE)

find_package(spdlog REQUIRED)
find_package(BZip2 REQUIRED)
find_package(ZLIB REQUIRED)
find_package(EXPAT REQUIRED)
include_directories(${OSMIUM_INCLUDE_DIRS})

FetchContent_Declare(
        raylib
        GIT_REPOSITORY https://github.com/raysan5/raylib.git
        GIT_TAG 5.5
        GIT_PROGRESS TRUE
        SYSTEM
)


FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/52eb8108c5bdec04579160ae17225d66034bd723.zip
        GIT_PROGRESS TRUE
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(googletest)
FetchContent_MakeAvailable(raylib)

# 1. Define raygui as a library so it's only compiled once
add_library(raygui_lib OBJECT vendor/raygui_impl.cpp)
target_include_directories(raygui_lib PRIVATE vendor/)
target_link_libraries(raygui_lib PRIVATE raylib)

enable_testing()

set(CORE_SOURCES
        src/Window.cpp
        src/Window.h
        src/raylib_logger.h
        src/path_finder.cpp
        src/path_finder.h
        src/user_interface.cpp
        src/user_interface.h
        src/models/dijkstra.cpp
        src/models/dijkstra.h
        src/models/pareto.cpp
        src/models/pareto.h
        src/models/a_star.cpp
        src/models/a_star.h
        src/models/weighted.cpp
        src/models/weighted.h
        src/osm/graph.cpp
        src/osm/graph.h
        src/osm/renderer.cpp
        src/osm/renderer.h
        vendor/raygui.h
        src/osm/osm_handler.cpp
        src/osm/osm_handler.h
        src/osm/osm_types.h
        src/timer.cpp
        src/timer.h
)

if (MSVC)
    add_compile_options(/W4 /permissive-)
    add_definitions(/DNOMINMAX)
else()
    add_compile_options(-Wall -Wextra -Wpedantic -O3)
endif()

# Main executable that runs the application

add_executable(
        priority_routing 
        src/main.cpp
        ${CORE_SOURCES}
)

target_include_directories(priority_routing PRIVATE vendor)

target_link_libraries(
        priority_routing
        PRIVATE
        raylib
        spdlog::spdlog
        BZip2::BZip2
        ZLIB::ZLIB
        EXPAT::EXPAT
        raygui_lib
)

# Test executable for testing

add_executable(
        priority_routing_tests
        tests/example_test.cpp
        ${CORE_SOURCES}
)

target_include_directories(priority_routing_tests PRIVATE vendor)

target_link_libraries(
        priority_routing_tests
        PRIVATE
        raylib
        spdlog::spdlog
        BZip2::BZip2
        ZLIB::ZLIB
        EXPAT::EXPAT
        GTest::gtest_main
        raygui_lib
)

include(GoogleTest)
gtest_discover_tests(priority_routing_tests)
