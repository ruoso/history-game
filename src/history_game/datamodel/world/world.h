#ifndef HISTORY_GAME_WORLD_H
#define HISTORY_GAME_WORLD_H

#include <vector>
#include <unordered_map>
#include <string>
#include <cpioo/managed_entity.hpp>
#include "datamodel/entity/entity.h"
#include "datamodel/npc/npc.h"
#include "datamodel/npc/npc_identity.h"
#include "datamodel/object/object.h"
#include "datamodel/world/simulation_clock.h"

namespace history_game {

/**
 * Structure that represents the world and manages all entities in the simulation
 */
struct World {
  // Simulation time tracking
  const SimulationClock::ref_type clock;
  
  // All NPCs in the world
  const std::vector<NPC::ref_type> npcs;
  
  // All objects in the world
  const std::vector<WorldObject::ref_type> objects;
  
  // Constructor
  World(
    const SimulationClock::ref_type& simulation_clock,
    std::vector<NPC::ref_type> world_npcs,
    std::vector<WorldObject::ref_type> world_objects
  ) : clock(simulation_clock),
      npcs(std::move(world_npcs)),
      objects(std::move(world_objects)) {}
      
  // Define storage type
  using storage = cpioo::managed_entity::storage<World, 4, uint16_t>;
  using ref_type = storage::ref_type;
};

} // namespace history_game

#endif // HISTORY_GAME_WORLD_H