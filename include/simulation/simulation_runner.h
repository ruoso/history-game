#ifndef HISTORY_GAME_SIMULATION_RUNNER_H
#define HISTORY_GAME_SIMULATION_RUNNER_H

#include <functional>
#include <spdlog/spdlog.h>
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
    bool new_gen = false;
    
    if (new_tick % clock->ticks_per_generation == 0) {
      new_generation++;
      new_gen = true;
    }
    
    // Create a new clock
    SimulationClock updated_clock(
      new_tick,
      new_generation,
      clock->ticks_per_generation
    );
    
    // Log the time advancement
    if (new_gen) {
      spdlog::info("Simulation advanced to tick {} (new generation {})", 
                  new_tick, new_generation);
    } else {
      spdlog::debug("Simulation advanced to tick {}", new_tick);
    }
    
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
    spdlog::info("Processing simulation tick {}", world->clock->current_tick);
    
    // 1. Update all NPCs (including action selection)
    spdlog::debug("Updating NPCs (count: {})", world->npcs.size());
    auto world_with_actions = npc_update_system::updateAllNPCs(world, params);
    
    // 2. Process perceptions based on the new actions
    spdlog::debug("Processing perceptions (range: {:.2f})", perception_range);
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
    
    spdlog::debug("Completed processing tick {}", world->clock->current_tick);
    
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
    spdlog::info("Starting simulation for {} ticks (initial tick: {})", 
                ticks, world->clock->current_tick);
    spdlog::info("World contains {} NPCs and {} objects", 
                world->npcs.size(), world->objects.size());
                
    World::ref_type current_world = world;
    
    for (uint64_t i = 0; i < ticks; ++i) {
      // Process one tick
      current_world = processTick(current_world, params, perception_range);
      
      // Call the callback if provided
      if (callback) {
        callback(current_world, i + 1);
      }
      
      // Log progress periodically for long simulations
      if (ticks > 10 && (i + 1) % (ticks / 10) == 0) {
        spdlog::info("Simulation progress: {:.0f}% ({}/{} ticks)", 
                    (i + 1) * 100.0 / ticks, i + 1, ticks);
      }
    }
    
    spdlog::info("Simulation complete - final tick: {}, generation: {}", 
                current_world->clock->current_tick,
                current_world->clock->current_generation);
    
    return current_world;
  }

} // namespace simulation_runner_system

} // namespace history_game

#endif // HISTORY_GAME_SIMULATION_RUNNER_H