#ifndef HISTORY_GAME_DATAMODEL_RELATIONSHIP_RELATIONSHIP_TARGET_H
#define HISTORY_GAME_DATAMODEL_RELATIONSHIP_RELATIONSHIP_TARGET_H

#include <variant>
#include <history_game/datamodel/entity/entity.h>
#include <history_game/datamodel/object/object.h>

namespace history_game::datamodel::relationship {

/**
 * Represents something that can be a target of a relationship:
 * - Another entity (NPC)
 * - A world object
 * - A location
 */
struct LocationPoint {
  const world::Position position;
  const float radius;
  
  LocationPoint(world::Position pos, float rad) : position(pos), radius(rad) {}
  
  bool contains(const world::Position& pos) const {
    float dx = pos.x - position.x;
    float dy = pos.y - position.y;
    float distance_squared = dx*dx + dy*dy;
    return distance_squared <= (radius * radius);
  }
};

// A variant that can hold any type of relationship target
using RelationshipTarget = std::variant<
  entity::Entity::ref_type,          // Another entity/NPC
  object::WorldObject::ref_type,     // A world object
  LocationPoint              // A location in the world
>;

namespace relationship_target_system {

  /**
   * Get the position of any relationship target
   */
  inline const world::Position& getPosition(const entity::Entity::ref_type& entity) {
    return entity->position;
  }

  inline const world::Position& getPosition(const object::WorldObject::ref_type& object) {
    return object->entity->position;
  }

  inline const world::Position& getPosition(const LocationPoint& location) {
    return location.position;
  }

  /**
   * Get the position of any relationship target using std::visit
   */
  inline const world::Position& getTargetPosition(const RelationshipTarget& target) {
    return std::visit([](const auto& t) -> const world::Position& { return getPosition(t); }, target);
  }

  /**
   * Check if a target contains a position (for proximity calculations)
   */
  inline bool contains(const entity::Entity::ref_type& entity, const world::Position& pos) {
    // Entities contain only their exact position
    const world::Position& entity_pos = entity->position;
    return entity_pos.x == pos.x && entity_pos.y == pos.y;
  }

  inline bool contains(const object::WorldObject::ref_type& object, const world::Position& pos) {
    // Objects contain only their exact position
    const world::Position& object_pos = object->entity->position;
    return object_pos.x == pos.x && object_pos.y == pos.y;
  }

  inline bool contains(const LocationPoint& location, const world::Position& pos) {
    // Locations contain any position within their radius
    return location.contains(pos);
  }

  /**
   * Check if any target contains a position using std::visit
   */
  inline bool targetContains(const RelationshipTarget& target, const world::Position& pos) {
    return std::visit([&pos](const auto& t) { return contains(t, pos); }, target);
  }

} // namespace relationship_target_system

} // namespace history_game::datamodel::relationship

#endif // HISTORY_GAME_DATAMODEL_RELATIONSHIP_RELATIONSHIP_TARGET_H