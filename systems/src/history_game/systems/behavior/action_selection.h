#ifndef HISTORY_GAME_SYSTEMS_BEHAVIOR_ACTION_SELECTION_H
#define HISTORY_GAME_SYSTEMS_BEHAVIOR_ACTION_SELECTION_H

#include <vector>
#include <optional>
#include <algorithm>
#include <random>
#include <cpioo/managed_entity.hpp>
#include <spdlog/spdlog.h>
#include <history_game/datamodel/npc/npc.h>
#include <history_game/datamodel/npc/drive.h>
#include <history_game/datamodel/action/action_type.h>
#include <history_game/datamodel/memory/memory_episode.h>
#include <history_game/datamodel/world/world.h>
#include <history_game/systems/drives/drive_impact.h>
#include <history_game/datamodel/object/object.h>

namespace history_game::systems::behavior {

/**
 * Represents a possible action that an NPC could take, with its potential targets
 */
struct ActionOption {
  // The action type
  const datamodel::action::ActionType action;
  
  // Possible targets for the action (if any)
  const std::optional<datamodel::entity::Entity::ref_type> target_entity;
  
  // Possible object target for the action (if any)
  const std::optional<datamodel::object::WorldObject::ref_type> target_object;
  
  // The expected drive impacts from performing this action
  const std::vector<datamodel::npc::Drive> expected_impacts;
  
  // Whether this action is from episodic memory or primitive heuristics
  const bool from_memory;
  
  // Constructor for action targeting an entity
  template<datamodel::action::ActionTypeConcept T>
  ActionOption(
    T action_type,
    const datamodel::entity::Entity::ref_type& entity,
    std::vector<datamodel::npc::Drive> impacts,
    bool is_from_memory = false
  ) : action(action_type),
      target_entity(entity),
      target_object(std::nullopt),
      expected_impacts(std::move(impacts)),
      from_memory(is_from_memory) {}
      
  // Constructor for action targeting an object
  template<datamodel::action::ActionTypeConcept T>
  ActionOption(
    T action_type,
    const datamodel::object::WorldObject::ref_type& object,
    std::vector<datamodel::npc::Drive> impacts,
    bool is_from_memory = false
  ) : action(action_type),
      target_entity(std::nullopt),
      target_object(object),
      expected_impacts(std::move(impacts)),
      from_memory(is_from_memory) {}
      
  // Constructor for action without a target
  template<datamodel::action::ActionTypeConcept T>
  ActionOption(
    T action_type,
    std::vector<datamodel::npc::Drive> impacts,
    bool is_from_memory = false
  ) : action(action_type),
      target_entity(std::nullopt),
      target_object(std::nullopt),
      expected_impacts(std::move(impacts)),
      from_memory(is_from_memory) {}
};

/**
 * Criteria for selecting an action
 */
struct ActionSelectionCriteria {
  // The NPC's current drives that need to be addressed
  const std::vector<datamodel::npc::Drive>& current_drives;
  
  // Preference for familiar actions vs. novel actions (0.0-1.0)
  const float familiarity_preference;
  
  // Preference for social vs. solitary actions (0.0-1.0)
  const float social_preference;
  
  // Random factor for non-deterministic behavior (0.0-1.0)
  const float randomness;
  
  // Constructor
  ActionSelectionCriteria(
    const std::vector<datamodel::npc::Drive>& drives,
    float f_pref = 0.5f,
    float s_pref = 0.5f,
    float rand = 0.2f
  ) : current_drives(drives),
      familiarity_preference(f_pref),
      social_preference(s_pref),
      randomness(rand) {}
};

namespace action_selection_system {

  // Helper function to get action type name for logging
  inline std::string get_action_name(const datamodel::action::ActionType& action) {
    return std::visit([](const auto& a) -> std::string { return a.name; }, action);
  }

  /**
   * Calculate a score for how well an action addresses the NPC's drives
   */
  inline float calculateDriveScore(
    const ActionOption& option,
    const std::vector<datamodel::npc::Drive>& current_drives
  ) {
    float total_score = 0.0f;
    
    // For each current drive, check if any expected impacts address it
    for (const auto& drive : current_drives) {
      // Skip drives with low intensity
      if (std::abs(drive.intensity) < 0.1f) {
        continue;
      }
      
      // Find any impacts that affect this drive
      for (const auto& impact : option.expected_impacts) {
        // Check if the impact addresses this drive
        if (drives::drive_impact_system::areSameDriveTypes(drive.type, impact.type)) {
          // Higher drive intensity and stronger impact give higher score
          // Negative impact intensity means drive reduction
          float drive_reduction = -impact.intensity * drive.intensity;
          total_score += drive_reduction;
        }
      }
    }
    
    return total_score;
  }
  
  /**
   * Calculate a score for how well an action matches the NPC's preferences
   */
  inline float calculatePreferenceScore(
    const ActionOption& option,
    const ActionSelectionCriteria& criteria
  ) {
    float pref_score = 0.0f;
    
    // Memory-based actions get a boost based on familiarity preference
    if (option.from_memory) {
      pref_score += criteria.familiarity_preference * 10.0f;
    }
    
    // Actions with entity targets get a boost based on social preference
    if (option.target_entity) {
      pref_score += criteria.social_preference * 5.0f;
    }
    
    return pref_score;
  }
  
  /**
   * Generate possible actions based on primitive drives (for bootstrapping behavior)
   */
  inline std::vector<ActionOption> generatePrimitiveActions(
    const datamodel::npc::NPC::ref_type& npc,
    const datamodel::world::World::ref_type& world
  ) {
    std::vector<ActionOption> options;
    const auto& npc_position = npc->identity->entity->position;
    
    // Find all nearby NPCs
    for (const auto& other_npc : world->npcs) {
      // Skip self
      if (other_npc->identity->entity->id == npc->identity->entity->id) {
        continue;
      }
      
      const auto& other_position = other_npc->identity->entity->position;
      float distance = std::sqrt(
        std::pow(npc_position.x - other_position.x, 2) +
        std::pow(npc_position.y - other_position.y, 2)
      );
      
      // Only consider NPCs within a certain range
      if (distance <= 10.0f) {
        // For Belonging drive: Follow
        options.emplace_back(
          datamodel::action::action_type::Follow{},
          other_npc->identity->entity,
          std::vector<datamodel::npc::Drive>{datamodel::npc::Drive(datamodel::npc::drive::Belonging{}, -0.3f)},
          false
        );
        
        // For Curiosity drive: Observe
        options.emplace_back(
          datamodel::action::action_type::Observe{},
          other_npc->identity->entity,
          std::vector<datamodel::npc::Drive>{datamodel::npc::Drive(datamodel::npc::drive::Curiosity{}, -0.2f)},
          false
        );
      }
    }
    
    // Find all nearby objects
    for (const auto& object : world->objects) {
      const auto& obj_position = object->entity->position;
      float distance = std::sqrt(
        std::pow(npc_position.x - obj_position.x, 2) +
        std::pow(npc_position.y - obj_position.y, 2)
      );
      
      // Only consider objects within a certain range
      if (distance <= 5.0f) {
        // For Curiosity drive: Observe object
        options.emplace_back(
          datamodel::action::action_type::Observe{},
          object,
          std::vector<datamodel::npc::Drive>{datamodel::npc::Drive(datamodel::npc::drive::Curiosity{}, -0.2f)},
          false
        );
        
        // Object-specific actions based on category
        std::visit([&](const auto& category) {
          using CategoryType = std::decay_t<decltype(category)>;
          
          if constexpr (std::is_same_v<CategoryType, datamodel::object::object_category::Food>) {
            // For Sustenance drive: Take food
            options.emplace_back(
              datamodel::action::action_type::Take{},
              object,
              std::vector<datamodel::npc::Drive>{datamodel::npc::Drive(datamodel::npc::drive::Sustenance{}, -0.5f)},
              false
            );
          }
          else if constexpr (std::is_same_v<CategoryType, datamodel::object::object_category::Structure>) {
            // For Shelter drive: Rest in structure
            options.emplace_back(
              datamodel::action::action_type::Rest{},
              object,
              std::vector<datamodel::npc::Drive>{
                datamodel::npc::Drive(datamodel::npc::drive::Shelter{}, -0.4f),
                datamodel::npc::Drive(datamodel::npc::drive::Sustenance{}, -0.3f)
              },
              false
            );
          }
        }, object->category);
      }
    }
    
    // Add untargeted actions
    
    // For Curiosity drive: Move
    options.emplace_back(
      datamodel::action::action_type::Move{},
      std::vector<datamodel::npc::Drive>{datamodel::npc::Drive(datamodel::npc::drive::Curiosity{}, -0.2f)},
      false
    );
    
    // For Shelter drive: Build
    options.emplace_back(
      datamodel::action::action_type::Build{},
      std::vector<datamodel::npc::Drive>{
        datamodel::npc::Drive(datamodel::npc::drive::Shelter{}, -0.3f),
        datamodel::npc::Drive(datamodel::npc::drive::Pride{}, -0.2f)
      },
      false
    );
    
    // For Pride drive: Gesture
    options.emplace_back(
      datamodel::action::action_type::Gesture{},
      std::vector<datamodel::npc::Drive>{datamodel::npc::Drive(datamodel::npc::drive::Pride{}, -0.3f)},
      false
    );
    
    return options;
  }
  
  /**
   * Generate possible actions from episodic memory
   */
  inline std::vector<ActionOption> generateMemoryBasedActions(
    const datamodel::npc::NPC::ref_type& npc,
    const datamodel::world::World::ref_type& world
  ) {
    std::vector<ActionOption> options;
    
    // Find episodic memories with significant positive impacts
    for (const auto& episode : npc->episodic_memory) {
      // Skip episodes that haven't been experienced multiple times
      if (episode->repetition_count < 2) {
        continue;
      }
      
      // Get the first action in the sequence (for simplicity)
      if (episode->action_sequence->steps.empty()) {
        continue;
      }
      
      const auto& first_step = episode->action_sequence->steps.front();
      const auto& memory = first_step.memory;
      
      // Use the action and targets from the memory
      datamodel::action::ActionType action = memory->action;
      std::optional<datamodel::entity::Entity::ref_type> target_entity = memory->target_entity;
      std::optional<datamodel::object::WorldObject::ref_type> target_object = memory->target_object;
      
      // Check if targets still exist in the world
      bool targets_exist = true;
      
      if (target_entity) {
        // Check if entity still exists (simplistic check)
        bool found = false;
        for (const auto& npc : world->npcs) {
          if (npc->identity->entity->id == target_entity.value()->id) {
            found = true;
            break;
          }
        }
        if (!found) targets_exist = false;
      }
      
      if (target_object) {
        // Check if object still exists (simplistic check)
        bool found = false;
        for (const auto& obj : world->objects) {
          if (obj->entity->id == target_object.value()->entity->id) {
            found = true;
            break;
          }
        }
        if (!found) targets_exist = false;
      }
      
      if (!targets_exist) {
        continue;
      }
      
      // Create action option with expected impacts from episode
      if (target_entity) {
        std::visit([&](const auto& specific_action) {
          options.emplace_back(
            specific_action,
            target_entity.value(),
            episode->drive_impacts,
            true  // From memory
          );
        }, action);
      }
      else if (target_object) {
        std::visit([&](const auto& specific_action) {
          options.emplace_back(
            specific_action,
            target_object.value(),
            episode->drive_impacts,
            true  // From memory
          );
        }, action);
      }
      else {
        // Untargeted action
        // Determine action type and use appropriate constructor
        std::visit([&](const auto& specific_action) {
          options.emplace_back(
            specific_action,
            episode->drive_impacts,
            true  // From memory
          );
        }, action);
      }
    }
    
    return options;
  }
  
  /**
   * Score and select an action from the available options
   */
  inline std::optional<ActionOption> selectAction(
    const std::vector<ActionOption>& options,
    const ActionSelectionCriteria& criteria
  ) {
    if (options.empty()) {
      return std::nullopt;
    }
    
    // Calculate scores for each option
    std::vector<std::pair<size_t, float>> scored_options;
    
    for (size_t i = 0; i < options.size(); ++i) {
      const auto& option = options[i];
      
      // Calculate drive satisfaction score
      float drive_score = calculateDriveScore(option, criteria.current_drives);
      
      // Calculate preference score
      float pref_score = calculatePreferenceScore(option, criteria);
      
      // Combine scores
      float total_score = drive_score + pref_score;
      
      scored_options.emplace_back(i, total_score);
    }
    
    // Sort by score (highest first)
    std::sort(scored_options.begin(), scored_options.end(),
      [](const auto& a, const auto& b) {
        return a.second > b.second;
      });
    
    // Apply randomness - sometimes choose from the top few options
    std::random_device rd;
    std::mt19937 gen(rd());
    
    if (criteria.randomness > 0.0f && scored_options.size() > 1) {
      float rand_threshold = criteria.randomness * 10.0f;
      size_t top_n = std::min(
        size_t(1 + rand_threshold),
        scored_options.size()
      );
      
      std::uniform_int_distribution<> dist(0, top_n - 1);
      size_t index = dist(gen);
      
      return options[scored_options[index].first];
    }
    
    // Otherwise choose the highest scoring option
    return options[scored_options.front().first];
  }
  
  /**
   * Update an NPC's identity to reflect a new action
   */
  inline datamodel::npc::NPCIdentity::ref_type updateIdentityWithAction(
    const datamodel::npc::NPCIdentity::ref_type& identity,
    const ActionOption& selected_action
  ) {
    // Log the action being taken
    const std::string action_name = get_action_name(selected_action.action);
    const std::string npc_id = identity->entity->id;
    
    if (selected_action.target_entity) {
      const std::string target_id = selected_action.target_entity.value()->id;
      spdlog::info("NPC {} performs {} targeting entity {}", npc_id, action_name, target_id);
    } 
    else if (selected_action.target_object) {
      const std::string object_id = selected_action.target_object.value()->entity->id;
      spdlog::info("NPC {} performs {} targeting object {}", npc_id, action_name, object_id);
    }
    else {
      spdlog::info("NPC {} performs {}", npc_id, action_name);
    }
    // Create a new identity with the updated action
    if (selected_action.target_entity) {
      // Action targeting an entity
      datamodel::npc::NPCIdentity updated_identity(
        identity->entity,
        selected_action.action,
        selected_action.target_entity.value()
      );
      return datamodel::npc::NPCIdentity::storage::make_entity(std::move(updated_identity));
    }
    else if (selected_action.target_object) {
      // Action targeting an object
      datamodel::npc::NPCIdentity updated_identity(
        identity->entity,
        selected_action.action,
        selected_action.target_object.value()
      );
      return datamodel::npc::NPCIdentity::storage::make_entity(std::move(updated_identity));
    }
    else {
      // Untargeted action
      datamodel::npc::NPCIdentity updated_identity(
        identity->entity,
        selected_action.action
      );
      return datamodel::npc::NPCIdentity::storage::make_entity(std::move(updated_identity));
    }
  }
  
  /**
   * The main function for selecting an NPC's next action
   * Returns an updated NPC with the new action
   */
  inline datamodel::npc::NPC::ref_type selectNextAction(
    const datamodel::npc::NPC::ref_type& npc,
    const datamodel::world::World::ref_type& world,
    const ActionSelectionCriteria& criteria
  ) {
    // Generate options from primitive drives
    auto primitive_options = generatePrimitiveActions(npc, world);
    
    // Generate options from episodic memory
    auto memory_options = generateMemoryBasedActions(npc, world);
    
    // Combine all options
    std::vector<ActionOption> all_options;
    all_options.reserve(primitive_options.size() + memory_options.size());
    
    // Add primitive options
    for (const auto& option : primitive_options) {
      all_options.push_back(option);
    }
    
    // Add memory options
    for (const auto& option : memory_options) {
      all_options.push_back(option);
    }
    
    // Select the best action
    auto selected_action = selectAction(all_options, criteria);
    
    if (!selected_action) {
      // No suitable action found, return unchanged NPC
      return npc;
    }
    
    // Update the NPC's identity with the selected action
    auto updated_identity = updateIdentityWithAction(
      npc->identity,
      selected_action.value()
    );
    
    // Create a new NPC with the updated identity
    datamodel::npc::NPC updated_npc(
      updated_identity,
      npc->drives,
      npc->perception,
      npc->episodic_memory,
      npc->observed_behaviors,
      npc->relationships
    );
    
    return datamodel::npc::NPC::storage::make_entity(std::move(updated_npc));
  }
  
  /**
   * Apply drive updates from taking an action
   * Returns an NPC with updated drive levels
   */
  inline datamodel::npc::NPC::ref_type applyDriveUpdates(
    const datamodel::npc::NPC::ref_type& npc,
    const ActionOption& action,
    float action_effectiveness = 1.0f
  ) {
    // Create a new vector for updated drives
    std::vector<datamodel::npc::Drive> updated_drives;
    updated_drives.reserve(npc->drives.size());
    
    // Track which drives have been updated
    std::vector<bool> updated(npc->drives.size(), false);
    
    // First pass: apply impacts
    for (size_t i = 0; i < npc->drives.size(); ++i) {
      const auto& drive = npc->drives[i];
      bool found_impact = false;
      
      // Check if this drive has a matching impact
      for (const auto& impact : action.expected_impacts) {
        if (drives::drive_impact_system::areSameDriveTypes(drive.type, impact.type)) {
          // Apply the impact, scaled by effectiveness
          float new_intensity = drive.intensity + (impact.intensity * action_effectiveness);
          
          // Ensure intensity stays within bounds (0-100)
          new_intensity = std::max(0.0f, std::min(100.0f, new_intensity));
          
          // Add updated drive
          updated_drives.emplace_back(drive.type, new_intensity);
          updated[i] = true;
          found_impact = true;
          break;
        }
      }
      
      // If no impact for this drive, copy the original
      if (!found_impact) {
        updated_drives.push_back(drive);
      }
    }
    
    
    // Create a new NPC with updated drives
    datamodel::npc::NPC updated_npc(
      npc->identity,
      updated_drives,
      npc->perception,
      npc->episodic_memory,
      npc->observed_behaviors,
      npc->relationships
    );
    
    return datamodel::npc::NPC::storage::make_entity(std::move(updated_npc));
  }

} // namespace action_selection_system

} // namespace history_game::systems::behavior

#endif // HISTORY_GAME_SYSTEMS_BEHAVIOR_ACTION_SELECTION_H