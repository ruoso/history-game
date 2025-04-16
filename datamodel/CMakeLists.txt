# Create library target
add_library(history_game_datamodel
  src/history_game/datamodel/action/action_sequence.cpp
  src/history_game/datamodel/action/action_sequence.h
  src/history_game/datamodel/action/action_type.cpp
  src/history_game/datamodel/action/action_type.h
  src/history_game/datamodel/drives/action_context.cpp
  src/history_game/datamodel/drives/action_context.h
  src/history_game/datamodel/entity/entity.cpp
  src/history_game/datamodel/entity/entity.h
  src/history_game/datamodel/memory/memory_entry.cpp
  src/history_game/datamodel/memory/memory_entry.h
  src/history_game/datamodel/memory/memory_episode.cpp
  src/history_game/datamodel/memory/memory_episode.h
  src/history_game/datamodel/memory/perception_buffer.cpp
  src/history_game/datamodel/memory/perception_buffer.h
  src/history_game/datamodel/memory/witnessed_sequence.cpp
  src/history_game/datamodel/memory/witnessed_sequence.h
  src/history_game/datamodel/npc/drive.cpp
  src/history_game/datamodel/npc/drive.h
  src/history_game/datamodel/npc/npc.cpp
  src/history_game/datamodel/npc/npc.h
  src/history_game/datamodel/npc/npc_identity.cpp
  src/history_game/datamodel/npc/npc_identity.h
  src/history_game/datamodel/object/object.cpp
  src/history_game/datamodel/object/object.h
  src/history_game/datamodel/relationship/relationship.cpp
  src/history_game/datamodel/relationship/relationship.h
  src/history_game/datamodel/relationship/relationship_target.cpp
  src/history_game/datamodel/relationship/relationship_target.h
  src/history_game/datamodel/world/position.cpp
  src/history_game/datamodel/world/position.h
  src/history_game/datamodel/world/simulation_clock.cpp
  src/history_game/datamodel/world/simulation_clock.h
  src/history_game/datamodel/world/world.cpp
  src/history_game/datamodel/world/world.h
)
target_include_directories(history_game_datamodel PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)
target_link_libraries(history_game_datamodel PUBLIC cpioo)

# Test configuration
enable_testing()

# Single test executable for all datamodel tests
add_executable(datamodel_tests
  tests/entity_test.cpp
)
target_link_libraries(datamodel_tests history_game_datamodel gtest gtest_main)
gtest_discover_tests(datamodel_tests)
