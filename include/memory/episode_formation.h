#ifndef HISTORY_GAME_EPISODE_FORMATION_H
#define HISTORY_GAME_EPISODE_FORMATION_H

#include <vector>
#include <algorithm>
#include <cpioo/managed_entity.hpp>
#include "npc/npc.h"
#include "memory/memory_entry.h"
#include "memory/memory_episode.h"
#include "memory/perception_buffer.h"
#include "action/action_sequence.h"
#include "drives/drive_impact.h"

namespace history_game {

namespace episode_formation_system {

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
    
    // Make a copy of the perceptions that we can sort
    std::vector<MemoryEntry::ref_type> sorted_perceptions = buffer->recent_perceptions;
    
    // Sort by timestamp
    std::sort(sorted_perceptions.begin(), sorted_perceptions.end(),
      [](const MemoryEntry::ref_type& a, const MemoryEntry::ref_type& b) {
        return a->timestamp < b->timestamp;
      });
    
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
              combined_impact = Drive(combined_impact.type, new_intensity);
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
    
    // No similar episode found
    return nullptr;
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
        
        if (similar_episode) {
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
    std::vector<MemoryEpisode::ref_type> updated_episodes = npc->episodic_memory;
    
    // Add new episodes
    updated_episodes.insert(
      updated_episodes.end(),
      new_episodes.begin(),
      new_episodes.end()
    );
    
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