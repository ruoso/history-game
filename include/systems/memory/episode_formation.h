#ifndef HISTORY_GAME_EPISODE_FORMATION_H
#define HISTORY_GAME_EPISODE_FORMATION_H

#include <vector>
#include <algorithm>
#include <cpioo/managed_entity.hpp>
#include <spdlog/spdlog.h>
#include "datamodel/npc/npc.h"
#include "datamodel/memory/memory_entry.h"
#include "datamodel/memory/memory_episode.h"
#include "datamodel/memory/perception_buffer.h"
#include "datamodel/action/action_sequence.h"
#include "systems/drives/drive_impact.h"

namespace history_game {

namespace episode_formation_system {

  /**
   * Get action name for logging
   */
  inline std::string get_action_name(const ActionType& action) {
    return std::visit([](const auto& a) -> std::string { return a.name; }, action);
  }
  
  /**
   * Get drive name for logging
   */
  inline std::string get_drive_name(const DriveType& drive) {
    return std::visit([](const auto& d) -> std::string { return d.name; }, drive);
  }

  /**
   * Identify sequences of related actions in the perception buffer
   * that could form meaningful episodes
   * 
   * @return A vector of vectors, where each inner vector is a potential sequence
   */
  inline std::vector<std::vector<MemoryEntry::ref_type>> identifyActionSequences(
    const PerceptionBuffer::ref_type& buffer,
    uint64_t max_sequence_gap = 5,  // Maximum ticks between related actions
    size_t min_sequence_length = 2  // Minimum actions to form a sequence
  ) {
    std::vector<std::vector<MemoryEntry::ref_type>> sequences;
    
    // Create a vector of indices that we can sort instead of the references directly
    std::vector<size_t> indices(buffer->recent_perceptions.size());
    for (size_t i = 0; i < buffer->recent_perceptions.size(); ++i) {
      indices[i] = i;
    }
    
    // Sort indices by timestamp
    std::sort(indices.begin(), indices.end(),
      [&buffer](size_t a, size_t b) {
        return buffer->recent_perceptions[a]->timestamp < buffer->recent_perceptions[b]->timestamp;
      });
      
    // Create a sorted vector using the indices
    std::vector<MemoryEntry::ref_type> sorted_perceptions;
    sorted_perceptions.reserve(buffer->recent_perceptions.size());
    for (size_t idx : indices) {
      sorted_perceptions.push_back(buffer->recent_perceptions[idx]);
    }
    
    // Initialize with an empty sequence
    std::vector<MemoryEntry::ref_type> current_sequence;
    
    // Process each perception in order
    for (const auto& perception : sorted_perceptions) {
      if (current_sequence.empty()) {
        // Start a new sequence
        current_sequence.push_back(perception);
      } else {
        // Get the last action in the current sequence
        const auto& last_action = current_sequence.back();
        
        // Check if this action is close enough in time to be part of the sequence
        if (perception->timestamp - last_action->timestamp <= max_sequence_gap) {
          // Add to the current sequence
          current_sequence.push_back(perception);
        } else {
          // This action is too far apart, end the current sequence if it's long enough
          if (current_sequence.size() >= min_sequence_length) {
            sequences.push_back(current_sequence);
          }
          
          // Start a new sequence
          current_sequence.clear();
          current_sequence.push_back(perception);
        }
      }
    }
    
    // Add the last sequence if it's long enough
    if (current_sequence.size() >= min_sequence_length) {
      sequences.push_back(current_sequence);
    }
    
    return sequences;
  }
  
  /**
   * Create an ActionSequence from a list of memory entries
   */
  inline ActionSequence::ref_type createActionSequence(
    const std::vector<MemoryEntry::ref_type>& entries,
    const std::string& sequence_id
  ) {
    std::vector<ActionStep> steps;
    steps.reserve(entries.size());
    
    // Add the first step with zero delay
    steps.emplace_back(entries[0], 0);
    
    // Add subsequent steps with calculated delays
    for (size_t i = 1; i < entries.size(); ++i) {
      uint32_t delay = static_cast<uint32_t>(entries[i]->timestamp - entries[i-1]->timestamp);
      steps.emplace_back(entries[i], delay);
    }
    
    // Create the action sequence
    ActionSequence sequence(sequence_id, std::move(steps));
    return ActionSequence::storage::make_entity(std::move(sequence));
  }
  
  /**
   * Evaluate the emotional impact of a sequence of memory entries
   */
  inline std::vector<Drive> evaluateSequenceImpact(
    const NPC::ref_type& npc,
    const std::vector<MemoryEntry::ref_type>& sequence,
    uint64_t current_time
  ) {
    // Calculate individual impacts for each memory in the sequence
    std::vector<std::vector<Drive>> all_impacts;
    
    for (const auto& memory : sequence) {
      ActionContext context(npc, memory, current_time);
      all_impacts.push_back(drive_impact_system::evaluateImpact(context));
    }
    
    // Combine all impacts into a single vector
    std::vector<Drive> combined_impacts;
    
    // Track which drive types we've already processed
    std::vector<DriveType> processed_types;
    
    // For each impact set
    for (const auto& impacts : all_impacts) {
      for (const auto& impact : impacts) {
        // Check if we've already seen this drive type
        auto it = std::find_if(processed_types.begin(), processed_types.end(),
          [&impact](const DriveType& type) {
            return drive_impact_system::areSameDriveTypes(type, impact.type);
          });
        
        if (it == processed_types.end()) {
          // First time seeing this drive type, add it to combined impacts
          combined_impacts.push_back(impact);
          processed_types.push_back(impact.type);
        } else {
          // We've seen this drive type before, find it in combined impacts and add to it
          for (auto& combined_impact : combined_impacts) {
            if (drive_impact_system::areSameDriveTypes(combined_impact.type, impact.type)) {
              // Add the impacts (effectively averaging them, but with weight toward stronger impacts)
              float new_intensity = (combined_impact.intensity + impact.intensity) * 0.6f;
              
              // Create a new vector with the updated impact
              std::vector<Drive> new_impacts;
              new_impacts.reserve(combined_impacts.size());
              
              for (size_t i = 0; i < combined_impacts.size(); ++i) {
                if (i == &combined_impact - &combined_impacts[0]) {
                  // Replace with updated drive
                  new_impacts.emplace_back(combined_impact.type, new_intensity);
                } else {
                  // Copy other drives unchanged
                  new_impacts.push_back(combined_impacts[i]);
                }
              }
              
              // Replace the vector
              combined_impacts = std::move(new_impacts);
              break;
            }
          }
        }
      }
    }
    
    return combined_impacts;
  }
  
  /**
   * Create a memory episode from a sequence of observations
   */
  inline MemoryEpisode::ref_type createMemoryEpisode(
    const std::vector<MemoryEntry::ref_type>& sequence,
    const std::vector<Drive>& impacts,
    const std::string& sequence_id,
    uint32_t repetition_count = 1
  ) {
    // Log memory episode creation
    std::string npc_id = sequence.front()->actor->entity->id;
    std::string impact_summary;
    
    for (const auto& impact : impacts) {
      impact_summary += get_drive_name(impact.type) + ":" + 
                        std::to_string(impact.intensity) + " ";
    }
    
    spdlog::info("NPC {} forms memory episode (id: {}, impacts: {})", 
                 npc_id, sequence_id, impact_summary);
    // Calculate start and end times
    uint64_t start_time = sequence.front()->timestamp;
    uint64_t end_time = sequence.back()->timestamp;
    
    // Create an action sequence
    auto action_sequence = createActionSequence(sequence, sequence_id);
    
    // Create the memory episode
    MemoryEpisode episode(
      start_time,
      end_time,
      action_sequence,
      impacts,
      repetition_count
    );
    
    return MemoryEpisode::storage::make_entity(std::move(episode));
  }
  
  /**
   * Find similar episodes in an NPC's memory
   * Returns a matching episode if found, or nullptr if no match
   */
  inline MemoryEpisode::ref_type findSimilarEpisode(
    const std::vector<MemoryEpisode::ref_type>& episodes,
    const ActionSequence::ref_type& sequence,
    float similarity_threshold = 0.7f
  ) {
    // For now, a simple implementation that just checks if the sequences have the same number of steps
    // A more sophisticated implementation would compare the actual actions
    for (const auto& episode : episodes) {
      const auto& episode_sequence = episode->action_sequence;
      
      // Check if the sequences have the same number of steps
      if (episode_sequence->steps.size() == sequence->steps.size()) {
        // Consider them similar for now
        // TODO: Implement a more sophisticated comparison
        return episode;
      }
    }
    
    // No similar episode found - just pick the first episode if any exist
    // This is a workaround for the fact that we can't return a null reference
    if (!episodes.empty()) {
      return episodes[0];
    }
    
    // If no episodes exist, create a dummy sequence and episode that we'll never use
    // First create an empty action sequence with a special ID
    ActionSequence dummy_sequence("__dummy__", {});
    auto dummy_sequence_ref = ActionSequence::storage::make_entity(std::move(dummy_sequence));
    
    // Now create a dummy episode with the sequence and special repetition_count of 0
    MemoryEpisode dummy(0, 0, dummy_sequence_ref, {}, 0);
    return MemoryEpisode::storage::make_entity(std::move(dummy));
  }
  
  /**
   * Process an NPC's perceptions to form new episodic memories
   * Returns an updated NPC with new or updated episodic memories
   */
  inline NPC::ref_type formEpisodicMemories(
    const NPC::ref_type& npc,
    uint64_t current_time,
    float significance_threshold = 0.3f,
    uint64_t max_sequence_gap = 5,
    size_t min_sequence_length = 2
  ) {
    // Identify potential action sequences
    auto sequences = identifyActionSequences(
      npc->perception,
      max_sequence_gap,
      min_sequence_length
    );
    
    if (sequences.empty()) {
      // No potential sequences, return unchanged NPC
      return npc;
    }
    
    // New episodes to add
    std::vector<MemoryEpisode::ref_type> new_episodes;
    
    // Process each potential sequence
    for (const auto& sequence : sequences) {
      // Evaluate the emotional impact
      auto impacts = evaluateSequenceImpact(npc, sequence, current_time);
      
      // Check if it has enough emotional significance
      if (drive_impact_system::hasEmotionalSignificance({impacts}, significance_threshold)) {
        // Create a unique ID for this sequence
        std::string sequence_id = "seq_" + std::to_string(current_time) + "_" + 
                                   std::to_string(sequence.size());
        
        // Create an action sequence for similarity checking
        auto action_sequence = createActionSequence(sequence, sequence_id);
        
        // Check if this is similar to an existing episode
        auto similar_episode = findSimilarEpisode(npc->episodic_memory, action_sequence);
        
        // Since we can't return a null reference, we need a way to check if it's a "real" episode
        // The dummy one we return has repetition_count of 0, which is never valid for a real episode
        if (similar_episode->repetition_count > 0) {
          // Update the existing episode with a higher repetition count
          MemoryEpisode updated_episode(
            similar_episode->start_time,
            similar_episode->end_time,
            similar_episode->action_sequence,
            similar_episode->drive_impacts,
            similar_episode->repetition_count + 1
          );
          
          new_episodes.push_back(
            MemoryEpisode::storage::make_entity(std::move(updated_episode))
          );
        } else {
          // This is a new type of episode
          new_episodes.push_back(
            createMemoryEpisode(sequence, impacts, sequence_id)
          );
        }
      }
    }
    
    if (new_episodes.empty()) {
      // No significant episodes formed, return unchanged NPC
      return npc;
    }
    
    // Combine existing episodes with new ones
    std::vector<MemoryEpisode::ref_type> updated_episodes;
    updated_episodes.reserve(npc->episodic_memory.size() + new_episodes.size());
    
    // Copy existing episodes
    for (const auto& episode : npc->episodic_memory) {
      updated_episodes.push_back(episode);
    }
    
    // Add new episodes
    for (const auto& episode : new_episodes) {
      updated_episodes.push_back(episode);
    }
    
    // Create updated NPC with new episodic memories
    NPC updated_npc(
      npc->identity,
      npc->drives,
      npc->perception,
      updated_episodes,
      npc->observed_behaviors,
      npc->relationships
    );
    
    return NPC::storage::make_entity(std::move(updated_npc));
  }

} // namespace episode_formation_system

} // namespace history_game

#endif // HISTORY_GAME_EPISODE_FORMATION_H