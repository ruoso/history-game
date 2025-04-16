#ifndef HISTORY_GAME_SIMULATION_RUNNER_H
#define HISTORY_GAME_SIMULATION_RUNNER_H

#include <functional>
#include <spdlog/spdlog.h>
#include <chrono>
#include <history_game/datamodel/world/world.h>
#include <history_game/systems/simulation/npc_update.h>
#include <history_game/systems/memory/memory_system.h>
#include <history_game/datamodel/world/simulation_clock.h>
#include <history_game/systems/utility/serialization.h>
#include <history_game/systems/action/action_execution.h>

namespace history_game {

// Use serialization's JSON type definition
using json = serialization::json;

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
    float perception_range = 10.0f,
    serialization::SimulationLogger* logger = nullptr
  ) {
    spdlog::info("Processing simulation tick {}", world->clock->current_tick);
    
    // Log tick start event if logger is provided
    if (logger && logger->isInitialized()) {
      uint64_t current_time = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
      
      logger->logEvent(serialization::createTickStartEvent(
        current_time, 
        world->clock->current_tick, 
        world->clock->current_generation
      ));
    }
    
    // 1. Update all NPCs (including action selection)
    spdlog::debug("Updating NPCs (count: {})", world->npcs.size());
    auto world_with_actions = npc_update_system::updateAllNPCs(world, params);

    // 2. Execute NPC actions
    spdlog::debug("Executing NPC actions");
    auto world_after_actions = action_execution_system::executeAllActions(world_with_actions, logger);
    
    // 3. Process perceptions based on the new actions
    spdlog::debug("Processing perceptions (range: {:.2f})", perception_range);
    auto world_with_perceptions = memory_system::processPerceptions(
      world_after_actions,
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
    
    auto result = World::storage::make_entity(std::move(updated_world));
    
    // Log tick end event and entity positions if logger is provided
    if (logger && logger->isInitialized()) {
      uint64_t current_time = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
      
      // Log tick end event
      logger->logEvent(serialization::createTickEndEvent(
        current_time, 
        world->clock->current_tick, 
        world->clock->current_generation,
        result->npcs.size(),
        result->objects.size()
      ));
      
      // Log entity updates - sample a few NPCs and objects to avoid too much data
      int npc_count = std::min(10, static_cast<int>(result->npcs.size()));
      int obj_count = std::min(10, static_cast<int>(result->objects.size()));
      
      // Log NPC positions and states
      for (int i = 0; i < npc_count; i++) {
        const auto& npc = result->npcs[i];
        
        // Create position JSON
        json position;
        position["x"] = npc->identity->entity->position.x;
        position["y"] = npc->identity->entity->position.y;
        
        // Create drives JSON
        json drives = json::array();
        for (const auto& drive : npc->drives) {
          json drive_json;
          drive_json["type"] = drive_dynamics_system::get_drive_name(drive.type);
          drive_json["value"] = drive.intensity;
          drives.push_back(drive_json);
        }
        
        // Get current action if any
        std::optional<std::string> action;
        if (npc->identity->current_action) {
          action = action_selection_system::get_action_name(npc->identity->current_action.value());
        }
        
        // Log entity update event
        logger->logEvent(serialization::createEntityUpdateEvent(
          current_time, 
          npc->identity->entity->id, 
          "NPC", 
          position, 
          drives, 
          action
        ));
      }
      
      // Log object positions
      for (int i = 0; i < obj_count; i++) {
        const auto& object = result->objects[i];
        
        // Create position JSON
        json position;
        position["x"] = object->entity->position.x;
        position["y"] = object->entity->position.y;
        
        // Log entity update event
        logger->logEvent(serialization::createEntityUpdateEvent(
          current_time, 
          object->entity->id, 
          "Object", 
          position
        ));
      }
    }
    
    spdlog::debug("Completed processing tick {}", world->clock->current_tick);
    
    return result;
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
  // Helper function to run a single tick
  inline World::ref_type runTick(
    const World::ref_type& world,
    const NPCUpdateParams& params,
    float perception_range,
    uint64_t tick_number,
    uint64_t total_ticks,
    const std::function<void(const World::ref_type&, uint64_t)>& callback,
    serialization::SimulationLogger* logger = nullptr
  ) {
    // Process one tick
    World::ref_type next_world = processTick(world, params, perception_range, logger);
    
    // Call the callback if provided
    if (callback) {
      callback(next_world, tick_number);
    }
    
    // Log progress periodically for long simulations
    if (total_ticks > 10 && tick_number % (total_ticks / 10) == 0) {
      spdlog::info("Simulation progress: {:.0f}% ({}/{} ticks)", 
                  tick_number * 100.0 / total_ticks, tick_number, total_ticks);
    }
    
    return next_world;
  }

  // Recursive implementation to avoid reassigning references
  inline World::ref_type runSimulationRecursive(
    const World::ref_type& world,
    uint64_t remaining_ticks,
    uint64_t total_ticks,
    uint64_t current_tick,
    const NPCUpdateParams& params,
    float perception_range,
    const std::function<void(const World::ref_type&, uint64_t)>& callback,
    serialization::SimulationLogger* logger = nullptr
  ) {
    if (remaining_ticks == 0) {
      return world;
    }
    
    // Process one tick
    World::ref_type next_world = runTick(world, params, perception_range, 
                                        current_tick, total_ticks, callback, logger);
    
    // Process remaining ticks recursively
    return runSimulationRecursive(next_world, remaining_ticks - 1, total_ticks, 
                                current_tick + 1, params, perception_range, callback, logger);
  }

  inline World::ref_type runSimulation(
    const World::ref_type& world,
    uint64_t ticks,
    const NPCUpdateParams& params,
    float perception_range = 10.0f,
    serialization::SimulationLogger* logger = nullptr,
    const std::function<void(const World::ref_type&, uint64_t)>& callback = nullptr
  ) {
    spdlog::info("Starting simulation for {} ticks (initial tick: {})", 
                ticks, world->clock->current_tick);
    spdlog::info("World contains {} NPCs and {} objects", 
                world->npcs.size(), world->objects.size());
    
    // Use recursion to avoid reassigning references
    World::ref_type final_world = runSimulationRecursive(world, ticks, ticks, 1, 
                                                        params, perception_range, callback, logger);
    
    spdlog::info("Simulation complete - final tick: {}, generation: {}", 
                final_world->clock->current_tick,
                final_world->clock->current_generation);
    
    return final_world;
  }

} // namespace simulation_runner_system

} // namespace history_game

#endif // HISTORY_GAME_SIMULATION_RUNNER_H