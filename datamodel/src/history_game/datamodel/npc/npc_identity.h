#ifndef HISTORY_GAME_DATAMODEL_NPC_NPC_IDENTITY_H
#define HISTORY_GAME_DATAMODEL_NPC_NPC_IDENTITY_H

#include <string>
#include <optional>
#include <cpioo/managed_entity.hpp>
#include <history_game/datamodel/entity/entity.h>
#include <history_game/datamodel/world/position.h>
#include <history_game/datamodel/action/action_type.h>

namespace history_game::datamodel::object {

struct WorldObject;

}
namespace history_game::datamodel::npc {

  // Forward declaration needed for the optional reference
  using WorldObjectRef = cpioo::managed_entity::reference<cpioo::managed_entity::storage<object::WorldObject, 10, uint32_t>>;

/**
 * Basic identity information for an NPC
 * This struct is referenced from memories and contains no references to memories,
 * preventing circular references
 */
struct NPCIdentity {
  // Reference to the base entity (contains ID and position)
  const entity::Entity::ref_type entity;
  
  // The action currently being performed
  const std::optional<action::ActionType> current_action;
  
  // Target of the action, if any (entity reference to avoid circular refs)
  const std::optional<entity::Entity::ref_type> target_entity;
  
  // Object involved in the action, if any
  const std::optional<WorldObjectRef> target_object;
  
  // Constructor with no action
  NPCIdentity(
    const entity::Entity::ref_type& entity_ref
  ) : entity(entity_ref),
      current_action(std::nullopt),
      target_entity(std::nullopt),
      target_object(std::nullopt) {}
  
  // Constructor with action targeting another entity
  template<action::ActionTypeConcept T>
  NPCIdentity(
    const entity::Entity::ref_type& entity_ref,
    T action,
    const entity::Entity::ref_type& target
  ) : entity(entity_ref),
      current_action(action),
      target_entity(target),
      target_object(std::nullopt) {}
      
  // Overload for ActionType variant with entity target
  NPCIdentity(
    const entity::Entity::ref_type& entity_ref,
    const action::ActionType& action,
    const entity::Entity::ref_type& target
  ) : entity(entity_ref),
      current_action(action),
      target_entity(target),
      target_object(std::nullopt) {}
  
  // Constructor with action targeting object
  template<action::ActionTypeConcept T>
  NPCIdentity(
    const entity::Entity::ref_type& entity_ref,
    T action,
    const WorldObjectRef& object
  ) : entity(entity_ref),
      current_action(action),
      target_entity(std::nullopt),
      target_object(object) {}
  
  // Overload for ActionType variant with object target
  NPCIdentity(
    const entity::Entity::ref_type& entity_ref,
    const action::ActionType& action,
    const WorldObjectRef& object
  ) : entity(entity_ref),
      current_action(action),
      target_entity(std::nullopt),
      target_object(object) {}
  
  // Constructor with untargeted action
  template<action::ActionTypeConcept T>
  NPCIdentity(
    const entity::Entity::ref_type& entity_ref,
    T action
  ) : entity(entity_ref),
      current_action(action),
      target_entity(std::nullopt),
      target_object(std::nullopt) {}
      
  // Overload for ActionType variant with no target
  NPCIdentity(
    const entity::Entity::ref_type& entity_ref,
    const action::ActionType& action
  ) : entity(entity_ref),
      current_action(action),
      target_entity(std::nullopt),
      target_object(std::nullopt) {}
      
  // Define storage type
  using storage = cpioo::managed_entity::storage<NPCIdentity, 10, uint32_t>;
  using ref_type = storage::ref_type;
};

} // namespace history_game::datamodel::npc

#endif // HISTORY_GAME_DATAMODEL_NPC_NPC_IDENTITY_H