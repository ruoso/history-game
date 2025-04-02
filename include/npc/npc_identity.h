#ifndef HISTORY_GAME_NPC_IDENTITY_H
#define HISTORY_GAME_NPC_IDENTITY_H

#include <string>
#include <cpioo/managed_entity.hpp>
#include "entity/entity.h"

namespace history_game {

/**
 * Basic identity information for an NPC
 * This struct is referenced from memories and contains no references to memories,
 * preventing circular references
 */
struct NPCIdentity {
  // Basic entity data
  const Entity entity;
  
  // A unique identifier for this NPC
  const std::string npc_id;
  
  // Constructor
  NPCIdentity(
    Entity entity_data,
    std::string id
  ) : entity(std::move(entity_data)),
      npc_id(std::move(id)) {}
      
  // Define storage type
  using storage = cpioo::managed_entity::storage<NPCIdentity, 10, uint32_t>;
  using ref_type = storage::ref_type;
};

} // namespace history_game

#endif // HISTORY_GAME_NPC_IDENTITY_H