# Create library target
add_library(history_game_systems
  src/history_game/systems/action/action_execution.cpp
  src/history_game/systems/action/action_execution.h
  src/history_game/systems/behavior/action_selection.cpp
  src/history_game/systems/behavior/action_selection.h
  src/history_game/systems/drives/drive_dynamics.cpp
  src/history_game/systems/drives/drive_dynamics.h
  src/history_game/systems/drives/drive_impact.cpp
  src/history_game/systems/drives/drive_impact.h
  src/history_game/systems/memory/episode_formation.cpp
  src/history_game/systems/memory/episode_formation.h
  src/history_game/systems/memory/memory_system.cpp
  src/history_game/systems/memory/memory_system.h
  src/history_game/systems/perception/perception_system.cpp
  src/history_game/systems/perception/perception_system.h
  src/history_game/systems/simulation/npc_update.cpp
  src/history_game/systems/simulation/npc_update.h
  src/history_game/systems/simulation/simulation_runner.cpp
  src/history_game/systems/simulation/simulation_runner.h
  src/history_game/systems/utility/log_init.cpp
  src/history_game/systems/utility/log_init.h
  src/history_game/systems/utility/serialization.cpp
  src/history_game/systems/utility/serialization.h
)
target_include_directories(history_game_systems PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)
target_link_libraries(history_game_systems PUBLIC history_game_datamodel spdlog::spdlog nlohmann_json::nlohmann_json)

# Test configuration
enable_testing()

# Single test executable for all systems tests
add_executable(systems_tests
  tests/drive_test.cpp
  tests/memory_test.cpp
  tests/serialization_test.cpp
)
target_link_libraries(systems_tests history_game_systems history_game_datamodel gtest gtest_main)
gtest_discover_tests(systems_tests)
