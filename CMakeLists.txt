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

add_subdirectory(datamodel)
add_subdirectory(systems)
add_subdirectory(bin)

# Visualizer test data generator
add_executable(generate_test_data visualizer/generate_test_data.cpp)
target_link_libraries(generate_test_data history_game_datamodel history_game_systems nlohmann_json::nlohmann_json)

