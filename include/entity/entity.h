#ifndef HISTORY_GAME_ENTITY_H
#define HISTORY_GAME_ENTITY_H

#include <string>
#include "world/position.h"

namespace history_game {

/**
 * Base entity struct for all simulation objects
 * Simple data container with const fields
 */
struct Entity {
  const std::string id;
  const Position position;
  
  // Constructor for initialization
  Entity(std::string entity_id, Position entity_position) 
    : id(std::move(entity_id)), position(entity_position) {}
};

} // namespace history_game

#endif // HISTORY_GAME_ENTITY_H