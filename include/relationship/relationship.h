#ifndef HISTORY_GAME_RELATIONSHIP_H
#define HISTORY_GAME_RELATIONSHIP_H

#include <cstdint>
#include <vector>
#include <cpioo/managed_entity.hpp>
#include "entity/entity.h"
#include "npc/drive.h"

namespace history_game {

/**
 * Represents emotional connection history for a drive
 */
struct AffectiveTrace {
  const DriveType drive_type;
  const float value;
  
  // Constructor
  template<DriveTypeConcept T>
  AffectiveTrace(T type, float trace_value)
    : drive_type(type), value(trace_value) {}
};

/**
 * Represents one NPC's relationship with another NPC
 * This is asymmetric - each NPC has their own perception of the relationship
 */
struct Relationship {
  // The target entity of this relationship
  const Entity::ref_type target;
  
  // Familiarity level (exposure)
  const float familiarity;
  
  // Emotional impact history per drive
  const std::vector<AffectiveTrace> affective_traces;
  
  // Last interaction timestamp
  const uint64_t last_interaction;
  
  // Number of shared episodic memories
  const uint32_t episodic_count;
  
  // Constructor
  Relationship(
    const Entity::ref_type& target_entity,
    float familiarity_level,
    std::vector<AffectiveTrace> traces,
    uint64_t interaction_time,
    uint32_t episode_count
  ) : target(target_entity),
      familiarity(familiarity_level),
      affective_traces(std::move(traces)),
      last_interaction(interaction_time),
      episodic_count(episode_count) {}
      
  // Define storage type
  using storage = cpioo::managed_entity::storage<Relationship, 10, uint32_t>;
  using ref_type = storage::ref_type;
};

} // namespace history_game

#endif // HISTORY_GAME_RELATIONSHIP_H