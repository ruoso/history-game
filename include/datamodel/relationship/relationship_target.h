#ifndef HISTORY_GAME_RELATIONSHIP_TARGET_H
#define HISTORY_GAME_RELATIONSHIP_TARGET_H

#include <variant>
#include "datamodel/entity/entity.h"
#include "datamodel/object/object.h"

namespace history_game {

/**
 * Represents something that can be a target of a relationship:
 * - Another entity (NPC)
 * - A world object
 * - A location
 */
struct LocationPoint {
  const Position position;
  const float radius;
  
  LocationPoint(Position pos, float rad) : position(pos), radius(rad) {}
  
  bool contains(const Position& pos) const {
    float dx = pos.x - position.x;
    float dy = pos.y - position.y;
    float distance_squared = dx*dx + dy*dy;
    return distance_squared <= (radius * radius);
  }
};

// A variant that can hold any type of relationship target
using RelationshipTarget = std::variant<
  Entity::ref_type,          // Another entity/NPC
  WorldObject::ref_type,     // A world object
  LocationPoint              // A location in the world
>;

namespace relationship_target_system {

  /**
   * Get the position of any relationship target
   */
  inline const Position& getPosition(const Entity::ref_type& entity) {
    return entity->position;
  }

  inline const Position& getPosition(const WorldObject::ref_type& object) {
    return object->entity->position;
  }

  inline const Position& getPosition(const LocationPoint& location) {
    return location.position;
  }

  /**
   * Get the position of any relationship target using std::visit
   */
  inline const Position& getTargetPosition(const RelationshipTarget& target) {
    return std::visit([](const auto& t) -> const Position& { return getPosition(t); }, target);
  }

  /**
   * Check if a target contains a position (for proximity calculations)
   */
  inline bool contains(const Entity::ref_type& entity, const Position& pos) {
    // Entities contain only their exact position
    const Position& entity_pos = entity->position;
    return entity_pos.x == pos.x && entity_pos.y == pos.y;
  }

  inline bool contains(const WorldObject::ref_type& object, const Position& pos) {
    // Objects contain only their exact position
    const Position& object_pos = object->entity->position;
    return object_pos.x == pos.x && object_pos.y == pos.y;
  }

  inline bool contains(const LocationPoint& location, const Position& pos) {
    // Locations contain any position within their radius
    return location.contains(pos);
  }

  /**
   * Check if any target contains a position using std::visit
   */
  inline bool targetContains(const RelationshipTarget& target, const Position& pos) {
    return std::visit([&pos](const auto& t) { return contains(t, pos); }, target);
  }

} // namespace relationship_target_system

} // namespace history_game

#endif // HISTORY_GAME_RELATIONSHIP_TARGET_H