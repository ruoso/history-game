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
FetchContent_MakeAvailable(spdlog)

# Fetch googletest for testing
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.14.0
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# Fetch inside-out-objects library
FetchContent_Declare(
    inside_out_objects
    GIT_REPOSITORY https://github.com/ruoso/inside-out-objects.git
    GIT_TAG main
)
FetchContent_MakeAvailable(inside_out_objects)

# Create library target
add_library(history_game_lib INTERFACE)
target_include_directories(history_game_lib INTERFACE 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${inside_out_objects_SOURCE_DIR}/include
)
target_link_libraries(history_game_lib INTERFACE spdlog::spdlog)

# Main executable
add_executable(history_game src/main.cpp)
target_link_libraries(history_game history_game_lib)

# Enable testing
enable_testing()

# Create test executable
add_executable(history_game_test 
    tests/entity_test.cpp
)
target_link_libraries(history_game_test 
    history_game_lib
    GTest::gtest_main
)

# Register tests
include(GoogleTest)
gtest_discover_tests(history_game_test)
