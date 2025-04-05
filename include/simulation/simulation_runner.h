#ifndef HISTORY_GAME_SIMULATION_RUNNER_H
#define HISTORY_GAME_SIMULATION_RUNNER_H

#include <functional>
#include "world/world.h"
#include "simulation/npc_update.h"
#include "memory/memory_system.h"
#include "world/simulation_clock.h"

namespace history_game {

namespace simulation_runner_system {

  /**
   * Advance the simulation clock by one tick
   */
  inline SimulationClock::ref_type advanceClock(
    const SimulationClock::ref_type& clock
  ) {
    // Create a new clock with incremented tick
    uint64_t new_tick = clock->current_tick + 1;
    
    // Determine if we should advance to a new generation
    uint32_t new_generation = clock->current_generation;
    if (new_tick % clock->ticks_per_generation == 0) {
      new_generation++;
    }
    
    // Create a new clock
    SimulationClock updated_clock(
      new_tick,
      new_generation,
      clock->ticks_per_generation
    );
    
    return SimulationClock::storage::make_entity(std::move(updated_clock));
  }
  
  /**
   * Process one complete simulation tick
   */
  inline World::ref_type processTick(
    const World::ref_type& world,
    const NPCUpdateParams& params,
    float perception_range = 10.0f
  ) {
    // 1. Update all NPCs (including action selection)
    auto world_with_actions = npc_update_system::updateAllNPCs(world, params);
    
    // 2. Process perceptions based on the new actions
    auto world_with_perceptions = memory_system::processPerceptions(
      world_with_actions,
      perception_range
    );
    
    // 3. Advance the simulation clock
    auto updated_clock = advanceClock(world_with_perceptions->clock);
    
    // 4. Create a new world with the updated clock
    World updated_world(
      updated_clock,
      world_with_perceptions->npcs,
      world_with_perceptions->objects
    );
    
    return World::storage::make_entity(std::move(updated_world));
  }
  
  /**
   * Run the simulation for a specified number of ticks
   * 
   * @param world The initial world state
   * @param ticks The number of ticks to simulate
   * @param params Parameters for NPC updates
   * @param perception_range The distance at which NPCs can perceive others
   * @param callback Optional callback to call after each tick
   * @return The final world state after all ticks
   */
  inline World::ref_type runSimulation(
    const World::ref_type& world,
    uint64_t ticks,
    const NPCUpdateParams& params,
    float perception_range = 10.0f,
    const std::function<void(const World::ref_type&, uint64_t)>& callback = nullptr
  ) {
    World::ref_type current_world = world;
    
    for (uint64_t i = 0; i < ticks; ++i) {
      // Process one tick
      current_world = processTick(current_world, params, perception_range);
      
      // Call the callback if provided
      if (callback) {
        callback(current_world, i + 1);
      }
    }
    
    return current_world;
  }

} // namespace simulation_runner_system

} // namespace history_game

#endif // HISTORY_GAME_SIMULATION_RUNNER_H