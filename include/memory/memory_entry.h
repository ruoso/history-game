#ifndef HISTORY_GAME_MEMORY_ENTRY_H
#define HISTORY_GAME_MEMORY_ENTRY_H

#include <string>
#include <cstdint>
#include <cpioo/managed_entity.hpp>
#include "entity/entity.h"
#include "npc/npc_identity.h"

namespace history_game {

/**
 * Represents a single observed action or event
 * These are the basic building blocks of NPC memory
 */
struct MemoryEntry {
  // When the memory was formed
  const uint64_t timestamp;
  
  // Who performed the action (if an NPC)
  const NPCIdentity::ref_type actor;
  
  // Where the action occurred
  const Position location;
  
  // Description of what was observed
  const std::string description;
  
  // Constructor
  MemoryEntry(
    uint64_t time, 
    const NPCIdentity::ref_type& actor_ref,
    Position memory_location,
    std::string memory_description
  ) : timestamp(time),
      actor(actor_ref),
      location(memory_location),
      description(std::move(memory_description)) {}
      
  // Define storage type
  using storage = cpioo::managed_entity::storage<MemoryEntry, 10, uint32_t>;
  using ref_type = storage::ref_type;
};

} // namespace history_game

#endif // HISTORY_GAME_MEMORY_ENTRY_H