#ifndef HISTORY_GAME_NPC_IDENTITY_H
#define HISTORY_GAME_NPC_IDENTITY_H

#include <string>
#include <optional>
#include <cpioo/managed_entity.hpp>
#include "entity/entity.h"
#include "world/position.h"
#include "action/action_type.h"

namespace history_game {

// Forward declaration needed for the optional reference
struct WorldObject;
using WorldObjectRef = cpioo::managed_entity::reference<cpioo::managed_entity::storage<WorldObject, 10, uint32_t>>;

/**
 * Basic identity information for an NPC
 * This struct is referenced from memories and contains no references to memories,
 * preventing circular references
 */
struct NPCIdentity {
  // Reference to the base entity (contains ID and position)
  const Entity::ref_type entity;
  
  // The action currently being performed
  const std::optional<ActionType> current_action;
  
  // Target of the action, if any (entity reference to avoid circular refs)
  const std::optional<Entity::ref_type> target_entity;
  
  // Object involved in the action, if any
  const std::optional<WorldObjectRef> target_object;
  
  // Constructor with no action
  NPCIdentity(
    const Entity::ref_type& entity_ref
  ) : entity(entity_ref),
      current_action(std::nullopt),
      target_entity(std::nullopt),
      target_object(std::nullopt) {}
  
  // Constructor with action targeting another entity
  template<ActionTypeConcept T>
  NPCIdentity(
    const Entity::ref_type& entity_ref,
    T action,
    const Entity::ref_type& target
  ) : entity(entity_ref),
      current_action(action),
      target_entity(target),
      target_object(std::nullopt) {}
      
  // Overload for ActionType variant with entity target
  NPCIdentity(
    const Entity::ref_type& entity_ref,
    const ActionType& action,
    const Entity::ref_type& target
  ) : entity(entity_ref),
      current_action(action),
      target_entity(target),
      target_object(std::nullopt) {}
  
  // Constructor with action targeting object
  template<ActionTypeConcept T>
  NPCIdentity(
    const Entity::ref_type& entity_ref,
    T action,
    const WorldObjectRef& object
  ) : entity(entity_ref),
      current_action(action),
      target_entity(std::nullopt),
      target_object(object) {}
  
  // Overload for ActionType variant with object target
  NPCIdentity(
    const Entity::ref_type& entity_ref,
    const ActionType& action,
    const WorldObjectRef& object
  ) : entity(entity_ref),
      current_action(action),
      target_entity(std::nullopt),
      target_object(object) {}
  
  // Constructor with untargeted action
  template<ActionTypeConcept T>
  NPCIdentity(
    const Entity::ref_type& entity_ref,
    T action
  ) : entity(entity_ref),
      current_action(action),
      target_entity(std::nullopt),
      target_object(std::nullopt) {}
      
  // Overload for ActionType variant with no target
  NPCIdentity(
    const Entity::ref_type& entity_ref,
    const ActionType& action
  ) : entity(entity_ref),
      current_action(action),
      target_entity(std::nullopt),
      target_object(std::nullopt) {}
      
  // Define storage type
  using storage = cpioo::managed_entity::storage<NPCIdentity, 10, uint32_t>;
  using ref_type = storage::ref_type;
};

} // namespace history_game

#endif // HISTORY_GAME_NPC_IDENTITY_H