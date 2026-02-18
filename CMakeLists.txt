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

# Enable Wayland support
set(USE_WAYLAND ON CACHE BOOL "Use Wayland" FORCE)

# Explicitly disable X11
set(USE_X11 OFF CACHE BOOL "Disable X11" FORCE)

# Optional: Disable GLFW X11 specifically if Raylib's internal GLFW needs it
set(GLFW_BUILD_X11 OFF CACHE BOOL "Disable GLFW X11" FORCE)
set(GLFW_BUILD_WAYLAND ON CACHE BOOL "Enable GLFW Wayland" FORCE)

FetchContent_MakeAvailable(raylib)

add_executable(priority_routing main.cpp
        Window.cpp
        Window.h)
target_link_libraries(
        priority_routing
        raylib
)