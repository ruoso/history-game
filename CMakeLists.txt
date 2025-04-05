cmake_minimum_required(VERSION 3.14)
project(history-game VERSION 0.1.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# Dependencies using FetchContent
include(FetchContent)

# Fetch spdlog for logging
FetchContent_Declare(
    spdlog
    GIT_REPOSITORY https://github.com/gabime/spdlog.git
    GIT_TAG v1.12.0
)

# Fetch inside-out-objects library
FetchContent_Declare(
    inside_out_objects
    GIT_REPOSITORY https://github.com/ruoso/poc-inside-out-objects.git
    GIT_TAG main
)

FetchContent_MakeAvailable(inside_out_objects spdlog)

# Create library target
add_library(history_game_lib INTERFACE)
target_include_directories(history_game_lib INTERFACE 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${inside_out_objects_SOURCE_DIR}/include
)
target_link_libraries(history_game_lib INTERFACE spdlog::spdlog cpioo)

# Main executable
add_executable(history_game src/main.cpp)
target_link_libraries(history_game history_game_lib)

