#ifndef HISTORY_GAME_DATAMODEL_MEMORY_MEMORY_ENTRY_H
#define HISTORY_GAME_DATAMODEL_MEMORY_MEMORY_ENTRY_H

#include <string>
#include <cstdint>
#include <optional>
#include <cpioo/managed_entity.hpp>
#include <history_game/datamodel/entity/entity.h>
#include <history_game/datamodel/npc/npc_identity.h>
#include <history_game/datamodel/action/action_type.h>
#include <history_game/datamodel/object/object.h>

namespace history_game::datamodel::memory {

/**
 * Represents a single observed action or event
 * These are the basic building blocks of NPC memory in the speechless world
 */
struct MemoryEntry {
  // When the memory was formed
  const uint64_t timestamp;
  
  // Who performed the action
  const npc::NPCIdentity::ref_type actor;
  
  // The action that was observed
  const action::ActionType action;
  
  // Target of the action, if any (optional)
  const std::optional<entity::Entity::ref_type> target_entity;
  
  // Object involved in the action, if any (optional)
  const std::optional<object::WorldObject::ref_type> target_object;
  
  // Constructor for action with entity target
  template<action::ActionTypeConcept T>
  MemoryEntry(
    uint64_t time, 
    const npc::NPCIdentity::ref_type& actor_ref,
    T action_type,
    const entity::Entity::ref_type& entity_target
  ) : timestamp(time),
      actor(actor_ref),
      action(action_type),
      target_entity(entity_target),
      target_object(std::nullopt) {}
  
  // Constructor for action with object target
  template<action::ActionTypeConcept T>
  MemoryEntry(
    uint64_t time, 
    const npc::NPCIdentity::ref_type& actor_ref,
    T action_type,
    const object::WorldObject::ref_type& object_target
  ) : timestamp(time),
      actor(actor_ref),
      action(action_type),
      target_entity(std::nullopt),
      target_object(object_target) {}
  
  // Constructor for action without a target
  template<action::ActionTypeConcept T>
  MemoryEntry(
    uint64_t time, 
    const npc::NPCIdentity::ref_type& actor_ref,
    T action_type
  ) : timestamp(time),
      actor(actor_ref),
      action(action_type),
      target_entity(std::nullopt),
      target_object(std::nullopt) {}
      
  // Define storage type
  using storage = cpioo::managed_entity::storage<MemoryEntry, 10, uint32_t>;
  using ref_type = storage::ref_type;
};

} // namespace history_game::datamodel::memory

#endif // HISTORY_GAME_DATAMODEL_MEMORY_MEMORY_ENTRY_H