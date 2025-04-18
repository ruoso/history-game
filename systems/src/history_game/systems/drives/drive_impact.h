#ifndef HISTORY_GAME_SYSTEMS_DRIVES_DRIVE_IMPACT_H
#define HISTORY_GAME_SYSTEMS_DRIVES_DRIVE_IMPACT_H

#include <vector>
#include <optional>
#include <string>
#include <variant>
#include <cpioo/managed_entity.hpp>
#include <history_game/datamodel/npc/drive.h>
#include <history_game/datamodel/npc/npc.h>
#include <history_game/datamodel/action/action_type.h>
#include <history_game/datamodel/memory/memory_entry.h>
#include <history_game/datamodel/relationship/relationship.h>
#include <history_game/datamodel/relationship/relationship_target.h>
#include <history_game/datamodel/drives/action_context.h>

namespace history_game::systems::drives {

namespace drive_impact_system {
  
  /**
   * Find any relationship the observer has with the action's actor
   */
  inline std::optional<datamodel::relationship::Relationship::ref_type> findActorRelationship(
    const datamodel::drives::ActionContext& context
  ) {
    // Get the actor's identity from the memory
    const datamodel::npc::NPCIdentity::ref_type& actor_identity = context.memory->actor;
    
    // Get the actor's entity reference
    const datamodel::entity::Entity::ref_type& actor_entity = actor_identity->entity;
    
    // Look for a relationship with this entity
    return datamodel::relationship::relationship_system::findRelationship(
      context.observer->relationships, 
      actor_entity
    );
  }
  
  /**
   * Find any relationship the observer has with the location of the action
   */
  inline std::optional<datamodel::relationship::Relationship::ref_type> findLocationRelationship(
    const datamodel::drives::ActionContext& context
  ) {
    // Get the memory's location where the action occurred
    const datamodel::entity::Entity::ref_type& location_entity = 
      context.memory->target_entity.value_or(context.memory->actor->entity);
    
    const datamodel::world::Position& action_position = location_entity->position;
    
    // Look for a relationship with this location
    return datamodel::relationship::relationship_system::findLocationRelationship(
      context.observer->relationships,
      action_position
    );
  }
  
  /**
   * Find any relationship the observer has with the object involved in the action
   */
  inline std::optional<datamodel::relationship::Relationship::ref_type> findObjectRelationship(
    const datamodel::drives::ActionContext& context
  ) {
    // Check if there's an object target
    if (!context.memory->target_object) {
      return std::nullopt;
    }
    
    // Get the object reference
    const datamodel::object::WorldObject::ref_type& target_object = context.memory->target_object.value();
    
    // Look for a relationship with this object
    return datamodel::relationship::relationship_system::findRelationship(
      context.observer->relationships,
      target_object
    );
  }
  
  /**
   * Get the familiarity level for a relationship
   */
  inline float getFamiliarity(const std::optional<datamodel::relationship::Relationship::ref_type>& relationship) {
    if (relationship) {
      return relationship.value()->familiarity;
    }
    return 0.0f; // No relationship means no familiarity
  }
  
  /**
   * Get the affective trace for a specific drive from a relationship
   */
  template<datamodel::npc::DriveTypeConcept T>
  inline float getAffectiveTrace(
    const std::optional<datamodel::relationship::Relationship::ref_type>& relationship,
    const T& drive_type
  ) {
    if (!relationship) {
      return 0.0f;
    }
    
    // Look for the specific drive in the affective traces
    for (const auto& trace : relationship.value()->affective_traces) {
      if (std::visit([](const auto& a, const auto& b) { 
          using A = std::decay_t<decltype(a)>;
          using B = std::decay_t<decltype(b)>;
          return std::is_same_v<A, B>; 
        }, trace.drive_type, datamodel::npc::DriveType{drive_type})) {
        return trace.value;
      }
    }
    
    return 0.0f;
  }
  
  /**
   * Helper function to compare two drive types
   */
  inline bool areSameDriveTypes(const datamodel::npc::DriveType& a, const datamodel::npc::DriveType& b) {
    return std::visit([](const auto& x, const auto& y) {
      using X = std::decay_t<decltype(x)>;
      using Y = std::decay_t<decltype(y)>;
      return std::is_same_v<X, Y>;
    }, a, b);
  }
  
  // Action-specific impact functions using ADL
  
  // Observe action impacts
  inline std::vector<datamodel::npc::Drive> getActionImpacts(
    const datamodel::action::action_type::Observe&, 
    const datamodel::drives::ActionContext& context
  ) {
    std::vector<datamodel::npc::Drive> impacts;
    
    // Get relationships
    auto actor_rel = findActorRelationship(context);
    auto location_rel = findLocationRelationship(context);
    
    // Base impact values
    float curiosity_impact = -0.1f; // Baseline reduction in curiosity
    
    // Modify impact based on familiarity
    float actor_familiarity = getFamiliarity(actor_rel);
    float location_familiarity = getFamiliarity(location_rel);
    
    // Less familiar things reduce curiosity more (satisfy it better)
    float familiarity_factor = 1.0f - ((actor_familiarity + location_familiarity) / 2.0f);
    curiosity_impact *= (1.0f + familiarity_factor);
    
    // Add the impact
    impacts.emplace_back(datamodel::npc::drive::Curiosity{}, curiosity_impact);
    
    return impacts;
  }
  
  // Follow action impacts
  inline std::vector<datamodel::npc::Drive> getActionImpacts(
    const datamodel::action::action_type::Follow&, 
    const datamodel::drives::ActionContext& context
  ) {
    std::vector<datamodel::npc::Drive> impacts;
    
    // Get actor relationship
    auto actor_rel = findActorRelationship(context);
    
    // Base impact values
    float belonging_impact = -0.2f; // Baseline reduction in belonging need
    
    // Modify impact based on familiarity
    float actor_familiarity = getFamiliarity(actor_rel);
    
    // More familiar NPCs provide stronger belonging satisfaction
    float familiarity_factor = actor_familiarity;
    belonging_impact *= (1.0f + familiarity_factor);
    
    // Add the impact
    impacts.emplace_back(datamodel::npc::drive::Belonging{}, belonging_impact);
    
    return impacts;
  }
  
  // Rest action impacts
  inline std::vector<datamodel::npc::Drive> getActionImpacts(
    const datamodel::action::action_type::Rest&, 
    const datamodel::drives::ActionContext& context
  ) {
    std::vector<datamodel::npc::Drive> impacts;
    
    // Get location relationship
    auto location_rel = findLocationRelationship(context);
    
    // Base impact values
    float sustenance_impact = -0.3f; // Baseline reduction in sustenance need
    
    // Modify impact based on location familiarity
    float location_familiarity = getFamiliarity(location_rel);
    
    // More familiar locations provide better rest
    float familiarity_factor = location_familiarity;
    sustenance_impact *= (1.0f + familiarity_factor);
    
    // Add the sustenance impact
    impacts.emplace_back(datamodel::npc::drive::Sustenance{}, sustenance_impact);
    
    // Familiar locations also provide shelter satisfaction
    if (location_familiarity > 0.3f) {
      impacts.emplace_back(datamodel::npc::drive::Shelter{}, -0.2f * location_familiarity);
    }
    
    return impacts;
  }
  
  // Default handler for actions without specific implementations
  template<typename T>
  inline std::vector<datamodel::npc::Drive> getActionImpacts(
    const T&, 
    const datamodel::drives::ActionContext& context
  ) {
    // Default implementation returns empty impacts
    return {};
  }
  
  /**
   * Adjust impacts based on current drive levels
   */
  inline std::vector<datamodel::npc::Drive> adjustImpacts(
    const std::vector<datamodel::npc::Drive>& impacts,
    const std::vector<datamodel::npc::Drive>& current_drives
  ) {
    std::vector<datamodel::npc::Drive> adjusted_impacts;
    adjusted_impacts.reserve(impacts.size());
    
    for (const auto& impact : impacts) {
      bool found_matching_drive = false;
      
      // Find the current level of this drive
      for (const auto& drive : current_drives) {
        if (areSameDriveTypes(impact.type, drive.type)) {
          // Higher drive intensity means higher impact magnitude
          float intensity_factor = drive.intensity / 100.0f;
          float new_intensity = impact.intensity * (1.0f + intensity_factor);
          adjusted_impacts.emplace_back(impact.type, new_intensity);
          found_matching_drive = true;
          break;
        }
      }
      
      // If no matching drive was found, just copy the original impact
      if (!found_matching_drive) {
        adjusted_impacts.push_back(impact);
      }
    }
    
    return adjusted_impacts;
  }

  /**
   * Evaluate how an observation impacts the observer's drives
   * using std::visit and argument-dependent lookup
   */
  inline std::vector<datamodel::npc::Drive> evaluateImpact(const datamodel::drives::ActionContext& context) {
    // Get base impacts using std::visit with ADL
    std::vector<datamodel::npc::Drive> base_impacts = std::visit(
      [&context](const auto& action_type) {
        return getActionImpacts(action_type, context);
      },
      context.memory->action
    );
    
    // Adjust impacts based on current drive levels
    return adjustImpacts(base_impacts, context.observer->drives);
  }
  
  /**
   * Determine if a sequence of observations has emotional significance
   * (i.e., is worth remembering as an episode)
   */
  inline bool hasEmotionalSignificance(
    const std::vector<std::vector<datamodel::npc::Drive>>& impacts,
    float significance_threshold = 0.5f
  ) {
    // Sum the total magnitude of impacts
    float total_magnitude = 0.0f;
    int total_impacts = 0;
    
    for (const auto& impact_set : impacts) {
      for (const auto& impact : impact_set) {
        total_magnitude += std::abs(impact.intensity);
        total_impacts++;
      }
    }
    
    // Calculate the average impact
    float avg_impact = 0.0f;
    if (total_impacts > 0) {
      avg_impact = total_magnitude / static_cast<float>(total_impacts);
    }
    
    // Consider significant if above threshold
    return avg_impact >= significance_threshold;
  }

} // namespace drive_impact_system

} // namespace history_game::systems::drives

#endif // HISTORY_GAME_SYSTEMS_DRIVES_DRIVE_IMPACT_H