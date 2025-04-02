cmake_minimum_required(VERSION 3.14)
project(history-game VERSION 0.1.0 LANGUAGES CXX)

# Set C++20 standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Include FetchContent for dependencies
include(FetchContent)

# Fetch inside-out-objects library
FetchContent_Declare(
  inside_out_objects
  GIT_REPOSITORY https://github.com/ruoso/poc-inside-out-objects.git
  GIT_TAG main
)

# Make the dependency available
FetchContent_MakeAvailable(inside_out_objects)

# Enable testing
enable_testing()

# For now, just create a simple test executable to inspect the library
add_executable(test_inside_out
  src/test_inside_out.cpp
)

target_link_libraries(test_inside_out
  PRIVATE
    cpioo
)