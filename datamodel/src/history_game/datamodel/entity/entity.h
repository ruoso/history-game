#ifndef HISTORY_GAME_DATAMODEL_ENTITY_ENTITY_H
#define HISTORY_GAME_DATAMODEL_ENTITY_ENTITY_H

#include <string>
#include <cpioo/managed_entity.hpp>
#include <history_game/datamodel/world/position.h>

namespace history_game::datamodel::entity {

/**
 * Base entity struct for all simulation objects
 * Simple data container with const fields
 */
struct Entity {
  const std::string id;
  const world::Position position;
  
  // Constructor for initialization
  Entity(std::string entity_id, world::Position entity_position) 
    : id(std::move(entity_id)), position(entity_position) {}
  
  // Define storage type
  using storage = cpioo::managed_entity::storage<Entity, 10, uint32_t>;
  using ref_type = storage::ref_type;
};

} // namespace history_game::datamodel::entity

#endif // HISTORY_GAME_DATAMODEL_ENTITY_ENTITY_H