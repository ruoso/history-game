#ifndef HISTORY_GAME_MEMORY_SYSTEM_H
#define HISTORY_GAME_MEMORY_SYSTEM_H

#include <vector>
#include <string>
#include <functional>
#include <unordered_map>
#include <spdlog/spdlog.h>
#include "world/world.h"
#include "npc/npc.h"
#include "memory/memory_entry.h"
#include "memory/perception_buffer.h"
#include "perception/perception_system.h"
#include "action/action_type.h"

namespace history_game {

namespace memory_system {

  /**
   * Get the NPCIdentity from an NPC
   */
  inline NPCIdentity::ref_type getIdentity(const NPC::ref_type& npc) {
    return npc->identity;
  }
  
  /**
   * Get the NPCIdentity from a WorldObject's creator
   */
  inline NPCIdentity::ref_type getIdentity(const WorldObject::ref_type& obj) {
    return obj->created_by;
  }

  /**
   * Create a memory entry for observing an NPC
   */
  inline MemoryEntry::ref_type createObservationEntry(
    uint64_t timestamp,
    const NPC::ref_type& observer,
    const NPC::ref_type& observed
  ) {
    // The observer is the full NPC, but we use its identity in the memory
    // The observed is also an NPC, but we store only its identity
    MemoryEntry entry(
      timestamp, 
      observer->identity,
      action_type::Observe{},
      observed->identity->entity
    );
    return MemoryEntry::storage::make_entity(std::move(entry));
  }
  
  /**
   * Create a memory entry for observing a world object
   */
  inline MemoryEntry::ref_type createObservationEntry(
    uint64_t timestamp,
    const NPC::ref_type& observer,
    const WorldObject::ref_type& observed
  ) {
    // The observer is the full NPC, but we use its identity in the memory
    MemoryEntry entry(
      timestamp, 
      observer->identity,
      action_type::Observe{},
      observed
    );
    return MemoryEntry::storage::make_entity(std::move(entry));
  }

  /**
   * Create a memory entry for an observed entity using argument-dependent lookup
   */
  inline MemoryEntry::ref_type createObservationMemory(
    const perception_system::PerceptionPair& perception,
    uint64_t timestamp
  ) {
    // The perceiver is the full NPC that will hold the memory
    const NPC::ref_type& perceiver = perception.perceiver;
    
    // Use std::visit with ADL to call the appropriate function
    return std::visit(
      [&](const auto& entity) {
        return createObservationEntry(timestamp, perceiver, entity);
      },
      perception.perceived
    );
  }
  
  /**
   * Create memory entries for all perceptions
   */
  inline std::vector<MemoryEntry::ref_type> createMemoryEntries(
    const std::vector<perception_system::PerceptionPair>& perceptions,
    uint64_t timestamp
  ) {
    std::vector<MemoryEntry::ref_type> entries;
    entries.reserve(perceptions.size());
    
    for (const auto& perception : perceptions) {
      entries.push_back(createObservationMemory(perception, timestamp));
    }
    
    return entries;
  }
  
  /**
   * Update an NPC's perception buffer with new memory entries
   */
  inline PerceptionBuffer::ref_type updatePerceptionBuffer(
    const PerceptionBuffer::ref_type& buffer,
    const std::vector<MemoryEntry::ref_type>& new_entries,
    size_t max_buffer_size = 20
  ) {
    // Combine existing entries with new ones
    std::vector<MemoryEntry::ref_type> updated_entries = buffer->recent_perceptions;
    
    // Add new entries
    updated_entries.insert(
      updated_entries.end(),
      new_entries.begin(),
      new_entries.end()
    );
    
    // Trim to max size if needed
    if (updated_entries.size() > max_buffer_size) {
      // Remove oldest entries (from the beginning of the vector)
      updated_entries.erase(
        updated_entries.begin(),
        updated_entries.begin() + (updated_entries.size() - max_buffer_size)
      );
    }
    
    // Create and return a new perception buffer
    PerceptionBuffer new_buffer(std::move(updated_entries));
    return PerceptionBuffer::storage::make_entity(std::move(new_buffer));
  }
  
  /**
   * Update an NPC with new perceptions
   */
  inline NPC::ref_type updateNPCPerceptions(
    const NPC::ref_type& npc,
    const std::vector<MemoryEntry::ref_type>& new_memories,
    size_t max_buffer_size = 20
  ) {
    // Update the perception buffer
    PerceptionBuffer::ref_type updated_buffer = 
      updatePerceptionBuffer(npc->perception, new_memories, max_buffer_size);
    
    // Create a new NPC with the updated perception buffer
    NPC updated_npc(
      npc->identity,
      npc->drives,
      updated_buffer,
      npc->episodic_memory,
      npc->observed_behaviors,
      npc->relationships
    );
    
    return NPC::storage::make_entity(std::move(updated_npc));
  }
  
  /**
   * Process all perceptions in the world and update NPCs' memory
   */
  inline World::ref_type processPerceptions(
    const World::ref_type& world,
    float perception_range = 10.0f,
    size_t max_buffer_size = 20
  ) {
    // Get the current time from the simulation clock
    uint64_t current_time = world->clock->current_tick;
    
    spdlog::debug("Processing perceptions at tick {} (range: {:.2f})", 
                 current_time, perception_range);
    
    // Calculate all perceptible entities in the world
    auto perceptions = perception_system::calculatePerceptibleEntities(
      world, 
      perception_range
    );
    
    spdlog::debug("Found {} perception events", perceptions.size());
    
    // Group perceptions by perceiver ID
    std::unordered_map<std::string, std::vector<MemoryEntry::ref_type>> npc_memories;
    
    // Create memory entries and group by NPC
    for (const auto& perception : perceptions) {
      auto memory = createObservationMemory(perception, current_time);
      npc_memories[perception_system::getId(perception.perceiver)].push_back(memory);
    }
    
    // Create updated NPCs with new memories
    std::vector<NPC::ref_type> updated_npcs;
    int npcs_with_perceptions = 0;
    
    for (const auto& npc : world->npcs) {
      const auto& npc_id = perception_system::getId(npc);
      
      // If this NPC perceived anything, update its perception buffer
      if (npc_memories.count(npc_id) > 0) {
        spdlog::trace("NPC {} received {} new perceptions", 
                     npc_id, npc_memories[npc_id].size());
                     
        auto updated_npc = updateNPCPerceptions(
          npc, 
          npc_memories[npc_id],
          max_buffer_size
        );
        updated_npcs.push_back(updated_npc);
        npcs_with_perceptions++;
      } else {
        // No new perceptions, keep the original NPC
        updated_npcs.push_back(npc);
      }
    }
    
    spdlog::debug("Updated perception buffers for {}/{} NPCs", 
                 npcs_with_perceptions, world->npcs.size());
    
    // Create a new world with updated NPCs
    World updated_world(
      world->clock,
      std::move(updated_npcs),
      world->objects
    );
    
    return World::storage::make_entity(std::move(updated_world));
  }

} // namespace memory_system

} // namespace history_game

#endif // HISTORY_GAME_MEMORY_SYSTEM_H