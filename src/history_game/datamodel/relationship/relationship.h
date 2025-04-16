#ifndef HISTORY_GAME_RELATIONSHIP_H
#define HISTORY_GAME_RELATIONSHIP_H

#include <cstdint>
#include <vector>
#include <cpioo/managed_entity.hpp>
#include "datamodel/entity/entity.h"
#include "datamodel/npc/drive.h"
#include "datamodel/relationship/relationship_target.h"

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
 * Represents one NPC's relationship with any target
 * (another NPC, a world object, or a location)
 */
struct Relationship {
  // The target of this relationship (can be entity, object, or location)
  const RelationshipTarget target;
  
  // Familiarity level (exposure)
  const float familiarity;
  
  // Emotional impact history per drive
  const std::vector<AffectiveTrace> affective_traces;
  
  // Last interaction timestamp
  const uint64_t last_interaction;
  
  // Number of interactions with this target
  const uint32_t interaction_count;
  
  // Constructor
  Relationship(
    RelationshipTarget relationship_target,
    float familiarity_level,
    std::vector<AffectiveTrace> traces,
    uint64_t interaction_time,
    uint32_t interactions
  ) : target(std::move(relationship_target)),
      familiarity(familiarity_level),
      affective_traces(std::move(traces)),
      last_interaction(interaction_time),
      interaction_count(interactions) {}
      
  // Define storage type
  using storage = cpioo::managed_entity::storage<Relationship, 10, uint32_t>;
  using ref_type = storage::ref_type;
};

namespace relationship_system {
  /**
   * Find a relationship with a specific target among a collection of relationships
   */
  template<typename T>
  inline std::optional<Relationship::ref_type> findRelationship(
    const std::vector<Relationship::ref_type>& relationships,
    const T& target_to_find
  ) {
    for (const auto& rel : relationships) {
      // Check if this relationship's target matches what we're looking for
      if (std::holds_alternative<T>(rel->target)) {
        // Get the target from the variant
        const T& target = std::get<T>(rel->target);
        
        // Check if it's the same entity
        if (target == target_to_find) {
          return rel;
        }
      }
    }
    
    // No matching relationship found
    return std::nullopt;
  }
  
  /**
   * Find a relationship with a location that contains the given position
   */
  inline std::optional<Relationship::ref_type> findLocationRelationship(
    const std::vector<Relationship::ref_type>& relationships,
    const Position& position
  ) {
    for (const auto& rel : relationships) {
      // Only check LocationPoint targets
      if (std::holds_alternative<LocationPoint>(rel->target)) {
        const LocationPoint& location = std::get<LocationPoint>(rel->target);
        
        // Check if the position is within this location
        if (location.contains(position)) {
          return rel;
        }
      }
    }
    
    // No matching location relationship found
    return std::nullopt;
  }
  
  /**
   * Check if an NPC is familiar with a specific target
   */
  template<typename T>
  inline bool isFamiliarWith(
    const std::vector<Relationship::ref_type>& relationships,
    const T& target,
    float familiarity_threshold = 0.5f
  ) {
    auto rel = findRelationship(relationships, target);
    if (rel) {
      return rel.value()->familiarity >= familiarity_threshold;
    }
    
    return false;
  }
  
  /**
   * Check if an NPC is familiar with a location
   */
  inline bool isFamiliarWithLocation(
    const std::vector<Relationship::ref_type>& relationships,
    const Position& position,
    float familiarity_threshold = 0.5f
  ) {
    auto rel = findLocationRelationship(relationships, position);
    if (rel) {
      return rel.value()->familiarity >= familiarity_threshold;
    }
    
    return false;
  }
}

} // namespace history_game

#endif // HISTORY_GAME_RELATIONSHIP_H