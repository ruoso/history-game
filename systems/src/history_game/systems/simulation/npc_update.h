#ifndef HISTORY_GAME_SYSTEMS_SIMULATION_NPC_UPDATE_H
#define HISTORY_GAME_SYSTEMS_SIMULATION_NPC_UPDATE_H

#include <vector>
#include <spdlog/spdlog.h>
#include <history_game/datamodel/world/world.h>
#include <history_game/datamodel/npc/npc.h>
#include <history_game/systems/drives/drive_dynamics.h>
#include <history_game/systems/behavior/action_selection.h>
#include <history_game/systems/memory/episode_formation.h>

namespace history_game {

/**
 * Parameters for NPC updates
 */
struct NPCUpdateParams {
  // Drive dynamics parameters
  const DriveParameters drive_params;
  
  // Action selection parameters
  const float familiarity_preference;
  const float social_preference;
  const float randomness;
  
  // Memory parameters
  const float significance_threshold;
  const uint64_t max_sequence_gap;
  const size_t min_sequence_length;
  
  // Constructor with default values
  NPCUpdateParams(
    DriveParameters drives = {},
    float f_pref = 0.5f,
    float s_pref = 0.5f,
    float rand = 0.2f,
    float sig_threshold = 0.3f,
    uint64_t max_gap = 5,
    size_t min_length = 2
  ) : drive_params(std::move(drives)),
      familiarity_preference(f_pref),
      social_preference(s_pref),
      randomness(rand),
      significance_threshold(sig_threshold),
      max_sequence_gap(max_gap),
      min_sequence_length(min_length) {}
};

namespace npc_update_system {

  /**
   * Update a single NPC for one simulation tick
   */
  inline NPC::ref_type updateNPC(
    const NPC::ref_type& npc,
    const World::ref_type& world,
    const NPCUpdateParams& params,
    uint64_t current_time
  ) {
    const std::string& npc_id = npc->identity->entity->id;
    
    // 1. Update drives based on natural increase
    auto npc_with_drives = drive_dynamics_system::updateDrives(
      npc,
      params.drive_params,
      1  // One tick has elapsed
    );
    
    // 2. Process perception to form episodic memories
    auto npc_with_memories = episode_formation_system::formEpisodicMemories(
      npc_with_drives,
      current_time,
      params.significance_threshold,
      params.max_sequence_gap,
      params.min_sequence_length
    );
    
    // 3. Select the next action based on drives and context
    ActionSelectionCriteria criteria(
      npc_with_memories->drives,
      params.familiarity_preference,
      params.social_preference,
      params.randomness
    );
    
    auto npc_with_action = action_selection_system::selectNextAction(
      npc_with_memories,
      world,
      criteria
    );
    
    return npc_with_action;
  }
  
  /**
   * Update all NPCs in the world for one simulation tick
   */
  inline World::ref_type updateAllNPCs(
    const World::ref_type& world,
    const NPCUpdateParams& params
  ) {
    // Get the current time from the simulation clock
    uint64_t current_time = world->clock->current_tick;
    
    spdlog::info("Updating all {} NPCs at tick {}", 
                world->npcs.size(), current_time);
    
    // Update each NPC
    std::vector<NPC::ref_type> updated_npcs;
    updated_npcs.reserve(world->npcs.size());
    
    for (const auto& npc : world->npcs) {
      updated_npcs.push_back(
        updateNPC(npc, world, params, current_time)
      );
    }
    
    // Create a new world with updated NPCs
    World updated_world(
      world->clock,
      std::move(updated_npcs),
      world->objects
    );
    
    spdlog::info("Completed updating all NPCs at tick {}", current_time);
    
    return World::storage::make_entity(std::move(updated_world));
  }

} // namespace npc_update_system

} // namespace history_game

#endif // HISTORY_GAME_SYSTEMS_SIMULATION_NPC_UPDATE_H