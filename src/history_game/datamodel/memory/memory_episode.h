#ifndef HISTORY_GAME_MEMORY_EPISODE_H
#define HISTORY_GAME_MEMORY_EPISODE_H

#include <cstdint>
#include <vector>
#include <string>
#include <cpioo/managed_entity.hpp>
#include "datamodel/action/action_sequence.h"
#include "datamodel/npc/drive.h"

namespace history_game {

/**
 * Represents the impact of an action sequence on an NPC's emotional drives
 */
struct MemoryEpisode {
  // Start and end time of the episode
  const uint64_t start_time;
  const uint64_t end_time;
  
  // Reference to the action sequence that created this episode
  const ActionSequence::ref_type action_sequence;
  
  // Impact on each drive
  const std::vector<Drive> drive_impacts;
  
  // How many times this has been repeated
  const uint32_t repetition_count;
  
  // Constructor
  MemoryEpisode(
    uint64_t start,
    uint64_t end,
    const ActionSequence::ref_type& sequence,
    std::vector<Drive> impacts,
    uint32_t repetitions = 1
  ) : start_time(start),
      end_time(end),
      action_sequence(sequence),
      drive_impacts(std::move(impacts)),
      repetition_count(repetitions) {}
      
  // Define storage type
  using storage = cpioo::managed_entity::storage<MemoryEpisode, 10, uint32_t>;
  using ref_type = storage::ref_type;
};

} // namespace history_game

#endif // HISTORY_GAME_MEMORY_EPISODE_H