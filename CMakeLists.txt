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

# Fetch nlohmann/json for serialization
FetchContent_Declare(
    json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.11.2
)

# Make dependencies available
FetchContent_MakeAvailable(inside_out_objects spdlog json)

# Enable testing (googletest is already fetched by inside_out_objects)
enable_testing()
include(GoogleTest)

# Create library target
add_library(history_game_lib
    src/utility/serialization.cpp
)
target_include_directories(history_game_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${inside_out_objects_SOURCE_DIR}/include
)
target_link_libraries(history_game_lib PUBLIC spdlog::spdlog cpioo nlohmann_json::nlohmann_json)

# Main executable
add_executable(history_game src/main.cpp)
target_link_libraries(history_game history_game_lib)

# Test executable for entity and related components
add_executable(entity_test tests/entity_test.cpp)
target_link_libraries(entity_test history_game_lib gtest gtest_main)
gtest_discover_tests(entity_test)

# Test executable for drive system
add_executable(drive_test tests/drive_test.cpp)
target_link_libraries(drive_test history_game_lib gtest gtest_main)
gtest_discover_tests(drive_test)

# Test executable for memory system
add_executable(memory_test tests/memory_test.cpp)
target_link_libraries(memory_test history_game_lib gtest gtest_main)
gtest_discover_tests(memory_test)

# Test serialization system
add_executable(serialization_test tests/serialization_test.cpp)
target_link_libraries(serialization_test history_game_lib gtest gtest_main)
gtest_discover_tests(serialization_test)

# Test inside-out objects integration
add_executable(inside_out_test src/test_inside_out.cpp)
target_link_libraries(inside_out_test history_game_lib gtest gtest_main)
gtest_discover_tests(inside_out_test)

# Visualizer test data generator
add_executable(generate_test_data visualizer/generate_test_data.cpp)
target_link_libraries(generate_test_data history_game_lib nlohmann_json::nlohmann_json)

