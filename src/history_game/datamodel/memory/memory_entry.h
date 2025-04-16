#ifndef HISTORY_GAME_MEMORY_ENTRY_H
#define HISTORY_GAME_MEMORY_ENTRY_H

#include <string>
#include <cstdint>
#include <optional>
#include <cpioo/managed_entity.hpp>
#include "datamodel/entity/entity.h"
#include "datamodel/npc/npc_identity.h"
#include "datamodel/action/action_type.h"
#include "datamodel/object/object.h"

namespace history_game {

/**
 * Represents a single observed action or event
 * These are the basic building blocks of NPC memory in the speechless world
 */
struct MemoryEntry {
  // When the memory was formed
  const uint64_t timestamp;
  
  // Who performed the action
  const NPCIdentity::ref_type actor;
  
  // The action that was observed
  const ActionType action;
  
  // Target of the action, if any (optional)
  const std::optional<Entity::ref_type> target_entity;
  
  // Object involved in the action, if any (optional)
  const std::optional<WorldObject::ref_type> target_object;
  
  // Constructor for action with entity target
  template<ActionTypeConcept T>
  MemoryEntry(
    uint64_t time, 
    const NPCIdentity::ref_type& actor_ref,
    T action_type,
    const Entity::ref_type& entity_target
  ) : timestamp(time),
      actor(actor_ref),
      action(action_type),
      target_entity(entity_target),
      target_object(std::nullopt) {}
  
  // Constructor for action with object target
  template<ActionTypeConcept T>
  MemoryEntry(
    uint64_t time, 
    const NPCIdentity::ref_type& actor_ref,
    T action_type,
    const WorldObject::ref_type& object_target
  ) : timestamp(time),
      actor(actor_ref),
      action(action_type),
      target_entity(std::nullopt),
      target_object(object_target) {}
  
  // Constructor for action without a target
  template<ActionTypeConcept T>
  MemoryEntry(
    uint64_t time, 
    const NPCIdentity::ref_type& actor_ref,
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

} // namespace history_game

#endif // HISTORY_GAME_MEMORY_ENTRY_H