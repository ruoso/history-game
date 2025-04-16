# Create library target
add_library(history_game_lib
  history_game/datamodel/action/action_sequence.cpp
  history_game/datamodel/action/action_sequence.h
  history_game/datamodel/action/action_type.cpp
  history_game/datamodel/action/action_type.h
  history_game/datamodel/drives/action_context.cpp
  history_game/datamodel/drives/action_context.h
  history_game/datamodel/entity/entity.cpp
  history_game/datamodel/entity/entity.h
  history_game/datamodel/memory/memory_entry.cpp
  history_game/datamodel/memory/memory_entry.h
  history_game/datamodel/memory/memory_episode.cpp
  history_game/datamodel/memory/memory_episode.h
  history_game/datamodel/memory/perception_buffer.cpp
  history_game/datamodel/memory/perception_buffer.h
  history_game/datamodel/memory/witnessed_sequence.cpp
  history_game/datamodel/memory/witnessed_sequence.h
  history_game/datamodel/npc/drive.cpp
  history_game/datamodel/npc/drive.h
  history_game/datamodel/npc/npc.cpp
  history_game/datamodel/npc/npc.h
  history_game/datamodel/npc/npc_identity.cpp
  history_game/datamodel/npc/npc_identity.h
  history_game/datamodel/object/object.cpp
  history_game/datamodel/object/object.h
  history_game/datamodel/relationship/relationship.cpp
  history_game/datamodel/relationship/relationship.h
  history_game/datamodel/relationship/relationship_target.cpp
  history_game/datamodel/relationship/relationship_target.h
  history_game/datamodel/world/position.cpp
  history_game/datamodel/world/position.h
  history_game/datamodel/world/simulation_clock.cpp
  history_game/datamodel/world/simulation_clock.h
  history_game/datamodel/world/world.cpp
  history_game/datamodel/world/world.h
  history_game/systems/action/action_execution.cpp
  history_game/systems/action/action_execution.h
  history_game/systems/behavior/action_selection.cpp
  history_game/systems/behavior/action_selection.h
  history_game/systems/drives/drive_dynamics.cpp
  history_game/systems/drives/drive_dynamics.h
  history_game/systems/drives/drive_impact.cpp
  history_game/systems/drives/drive_impact.h
  history_game/systems/memory/episode_formation.cpp
  history_game/systems/memory/episode_formation.h
  history_game/systems/memory/memory_system.cpp
  history_game/systems/memory/memory_system.h
  history_game/systems/perception/perception_system.cpp
  history_game/systems/perception/perception_system.h
  history_game/systems/simulation/npc_update.cpp
  history_game/systems/simulation/npc_update.h
  history_game/systems/simulation/simulation_runner.cpp
  history_game/systems/simulation/simulation_runner.h
  history_game/systems/utility/log_init.cpp
  history_game/systems/utility/log_init.h
  history_game/systems/utility/serialization.cpp
  history_game/systems/utility/serialization.h
)
target_include_directories(history_game_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)
target_link_libraries(history_game_lib PUBLIC spdlog::spdlog cpioo nlohmann_json::nlohmann_json)

# Main executable
add_executable(history_game main.cpp)
target_link_libraries(history_game history_game_lib)

