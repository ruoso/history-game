#ifndef HISTORY_GAME_MEMORY_EPISODE_H
#define HISTORY_GAME_MEMORY_EPISODE_H

#include <cstdint>
#include <vector>
#include <string>
#include <cpioo/managed_entity.hpp>
#include "action/action_sequence.h"
#include "npc/drive.h"

namespace history_game {

/**
 * Represents the emotional impact of an action sequence
 */
struct DriveImpact {
  const DriveType type;
  const float delta;
  
  // Constructor
  template<DriveTypeConcept T>
  DriveImpact(T drive_type, float impact_value)
    : type(drive_type), delta(impact_value) {}
};

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
  const std::vector<DriveImpact> drive_impacts;
  
  // How many times this has been repeated
  const uint32_t repetition_count;
  
  // Constructor
  MemoryEpisode(
    uint64_t start,
    uint64_t end,
    const ActionSequence::ref_type& sequence,
    std::vector<DriveImpact> impacts,
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