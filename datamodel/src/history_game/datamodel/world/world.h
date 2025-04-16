#ifndef HISTORY_GAME_DATAMODEL_WORLD_WORLD_H
#define HISTORY_GAME_DATAMODEL_WORLD_WORLD_H

#include <vector>
#include <unordered_map>
#include <string>
#include <cpioo/managed_entity.hpp>
#include <history_game/datamodel/entity/entity.h>
#include <history_game/datamodel/npc/npc.h>
#include <history_game/datamodel/npc/npc_identity.h>
#include <history_game/datamodel/object/object.h>
#include <history_game/datamodel/world/simulation_clock.h>

namespace history_game::datamodel::world {

/**
 * Structure that represents the world and manages all entities in the simulation
 */
struct World {
  // Simulation time tracking
  const SimulationClock::ref_type clock;
  
  // All NPCs in the world
  const std::vector<npc::NPC::ref_type> npcs;
  
  // All objects in the world
  const std::vector<object::WorldObject::ref_type> objects;
  
  // Constructor
  World(
    const SimulationClock::ref_type& simulation_clock,
    std::vector<npc::NPC::ref_type> world_npcs,
    std::vector<object::WorldObject::ref_type> world_objects
  ) : clock(simulation_clock),
      npcs(std::move(world_npcs)),
      objects(std::move(world_objects)) {}
      
  // Define storage type
  using storage = cpioo::managed_entity::storage<World, 4, uint16_t>;
  using ref_type = storage::ref_type;
};

} // namespace history_game::datamodel::world

#endif // HISTORY_GAME_DATAMODEL_WORLD_WORLD_H