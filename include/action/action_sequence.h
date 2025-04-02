#ifndef HISTORY_GAME_ACTION_SEQUENCE_H
#define HISTORY_GAME_ACTION_SEQUENCE_H

#include <vector>
#include <string>
#include <cstdint>
#include <cpioo/managed_entity.hpp>
#include "memory/memory_entry.h"

namespace history_game {

/**
 * Represents a step in an action sequence
 */
struct ActionStep {
  // The memory entry for this action step
  const MemoryEntry::ref_type memory;
  
  // Delay in ticks after the previous step
  const uint32_t delay_after_previous;
  
  // Constructor
  ActionStep(
    const MemoryEntry::ref_type& memory_ref,
    uint32_t delay
  ) : memory(memory_ref),
      delay_after_previous(delay) {}
};

/**
 * Represents a sequence of related actions that form a meaningful behavior
 */
struct ActionSequence {
  // Unique identifier for this sequence
  const std::string id;
  
  // The ordered list of action steps
  const std::vector<ActionStep> steps;
  
  // Constructor
  ActionSequence(
    std::string sequence_id,
    std::vector<ActionStep> action_steps
  ) : id(std::move(sequence_id)),
      steps(std::move(action_steps)) {}
      
  // Define storage type
  using storage = cpioo::managed_entity::storage<ActionSequence, 10, uint32_t>;
  using ref_type = storage::ref_type;
};

} // namespace history_game

#endif // HISTORY_GAME_ACTION_SEQUENCE_H