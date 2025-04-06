#ifndef HISTORY_GAME_PERCEPTION_BUFFER_H
#define HISTORY_GAME_PERCEPTION_BUFFER_H

#include <vector>
#include <cstdint>
#include <cpioo/managed_entity.hpp>
#include "datamodel/memory/memory_entry.h"

namespace history_game {

/**
 * Short-term buffer of recent observations and actions
 * This is the working memory of an NPC
 */
struct PerceptionBuffer {
  // List of recent memory entries
  const std::vector<MemoryEntry::ref_type> recent_perceptions;
  
  // Constructor
  explicit PerceptionBuffer(
    std::vector<MemoryEntry::ref_type> perceptions
  ) : recent_perceptions(std::move(perceptions)) {}
      
  // Define storage type
  using storage = cpioo::managed_entity::storage<PerceptionBuffer, 10, uint32_t>;
  using ref_type = storage::ref_type;
};

} // namespace history_game

#endif // HISTORY_GAME_PERCEPTION_BUFFER_H