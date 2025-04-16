#ifndef HISTORY_GAME_SYSTEMS_MEMORY_MEMORY_SYSTEM_H
#define HISTORY_GAME_SYSTEMS_MEMORY_MEMORY_SYSTEM_H

#include <vector>
#include <string>
#include <functional>
#include <unordered_map>
#include <spdlog/spdlog.h>
#include <history_game/datamodel/world/world.h>
#include <history_game/datamodel/npc/npc.h>
#include <history_game/datamodel/memory/memory_entry.h>
#include <history_game/datamodel/memory/perception_buffer.h>
#include <history_game/systems/perception/perception_system.h>
#include <history_game/datamodel/action/action_type.h>

namespace history_game::systems::memory {

  /**
   * Get the NPCIdentity from an NPC
   */
  inline datamodel::npc::NPCIdentity::ref_type getIdentity(const datamodel::npc::NPC::ref_type& npc) {
    return npc->identity;
  }
  
  /**
   * Get the NPCIdentity from a WorldObject's creator
   */
  inline datamodel::npc::NPCIdentity::ref_type getIdentity(const datamodel::object::WorldObject::ref_type& obj) {
    return obj->created_by;
  }

  /**
   * Create a memory entry for observing an NPC
   */
  inline datamodel::memory::MemoryEntry::ref_type createObservationEntry(
    uint64_t timestamp,
    const datamodel::npc::NPC::ref_type& observer,
    const datamodel::npc::NPC::ref_type& observed
  ) {
    // The observer is the full NPC, but we use its identity in the memory
    // The observed is also an NPC, but we store only its identity
    datamodel::memory::MemoryEntry entry(
      timestamp, 
      observer->identity,
      datamodel::action::action_type::Observe{},
      observed->identity->entity
    );
    return datamodel::memory::MemoryEntry::storage::make_entity(std::move(entry));
  }
  
  /**
   * Create a memory entry for observing a world object
   */
  inline datamodel::memory::MemoryEntry::ref_type createObservationEntry(
    uint64_t timestamp,
    const datamodel::npc::NPC::ref_type& observer,
    const datamodel::object::WorldObject::ref_type& observed
  ) {
    // The observer is the full NPC, but we use its identity in the memory
    datamodel::memory::MemoryEntry entry(
      timestamp, 
      observer->identity,
      datamodel::action::action_type::Observe{},
      observed
    );
    return datamodel::memory::MemoryEntry::storage::make_entity(std::move(entry));
  }

  /**
   * Create a memory entry for an observed entity using argument-dependent lookup
   */
  inline datamodel::memory::MemoryEntry::ref_type createObservationMemory(
    const perception::PerceptionPair& perception,
    uint64_t timestamp
  ) {
    // The perceiver is the full NPC that will hold the memory
    const datamodel::npc::NPC::ref_type& perceiver = perception.perceiver;
    
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
  inline std::vector<datamodel::memory::MemoryEntry::ref_type> createMemoryEntries(
    const std::vector<perception::PerceptionPair>& perceptions,
    uint64_t timestamp
  ) {
    std::vector<datamodel::memory::MemoryEntry::ref_type> entries;
    entries.reserve(perceptions.size());
    
    for (const auto& perception : perceptions) {
      entries.push_back(createObservationMemory(perception, timestamp));
    }
    
    return entries;
  }
  
  /**
   * Update an NPC's perception buffer with new memory entries
   */
  inline datamodel::memory::PerceptionBuffer::ref_type updatePerceptionBuffer(
    const datamodel::memory::PerceptionBuffer::ref_type& buffer,
    const std::vector<datamodel::memory::MemoryEntry::ref_type>& new_entries,
    size_t max_buffer_size = 20
  ) {
    // Combine existing entries with new ones
    std::vector<datamodel::memory::MemoryEntry::ref_type> updated_entries;
    updated_entries.reserve(buffer->recent_perceptions.size() + new_entries.size());
    
    // Copy existing entries
    for (const auto& entry : buffer->recent_perceptions) {
      updated_entries.push_back(entry);
    }
    
    // Add new entries
    for (const auto& entry : new_entries) {
      updated_entries.push_back(entry);
    }
    
    // Trim to max size if needed
    if (updated_entries.size() > max_buffer_size) {
      // Create a new vector with only the most recent entries
      std::vector<datamodel::memory::MemoryEntry::ref_type> trimmed_entries;
      trimmed_entries.reserve(max_buffer_size);
      
      // Copy only the newest entries (from the end of the vector)
      for (size_t i = updated_entries.size() - max_buffer_size; i < updated_entries.size(); ++i) {
        trimmed_entries.push_back(updated_entries[i]);
      }
      
      // Replace with the trimmed vector
      updated_entries = std::move(trimmed_entries);
    }
    
    // Create and return a new perception buffer
    datamodel::memory::PerceptionBuffer new_buffer(std::move(updated_entries));
    return datamodel::memory::PerceptionBuffer::storage::make_entity(std::move(new_buffer));
  }
  
  /**
   * Update an NPC with new perceptions
   */
  inline datamodel::npc::NPC::ref_type updateNPCPerceptions(
    const datamodel::npc::NPC::ref_type& npc,
    const std::vector<datamodel::memory::MemoryEntry::ref_type>& new_memories,
    size_t max_buffer_size = 20
  ) {
    // Update the perception buffer
    datamodel::memory::PerceptionBuffer::ref_type updated_buffer = 
      updatePerceptionBuffer(npc->perception, new_memories, max_buffer_size);
    
    // Create a new NPC with the updated perception buffer
    datamodel::npc::NPC updated_npc(
      npc->identity,
      npc->drives,
      updated_buffer,
      npc->episodic_memory,
      npc->observed_behaviors,
      npc->relationships
    );
    
    return datamodel::npc::NPC::storage::make_entity(std::move(updated_npc));
  }
  
  /**
   * Process all perceptions in the world and update NPCs' memory
   */
  inline datamodel::world::World::ref_type processPerceptions(
    const datamodel::world::World::ref_type& world,
    float perception_range = 10.0f,
    size_t max_buffer_size = 20
  ) {
    // Get the current time from the simulation clock
    uint64_t current_time = world->clock->current_tick;
        
    // Calculate all perceptible entities in the world
    auto perceptions = perception::calculatePerceptibleEntities(
      world, 
      perception_range
    );
    
    if (perceptions.size())
      spdlog::debug("Found {} perception events", perceptions.size());
    
    // Group perceptions by perceiver ID
    std::unordered_map<std::string, std::vector<datamodel::memory::MemoryEntry::ref_type>> npc_memories;
    
    // Create memory entries and group by NPC
    for (const auto& perception : perceptions) {
      auto memory = createObservationMemory(perception, current_time);
      npc_memories[perception::getId(perception.perceiver)].push_back(memory);
    }
    
    // Create updated NPCs with new memories
    std::vector<datamodel::npc::NPC::ref_type> updated_npcs;
    int npcs_with_perceptions = 0;
    
    for (const auto& npc : world->npcs) {
      const auto& npc_id = perception::getId(npc);
      
      // If this NPC perceived anything, update its perception buffer
      if (npc_memories.count(npc_id) > 0) {
                     
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
        
    // Create a new world with updated NPCs
    datamodel::world::World updated_world(
      world->clock,
      std::move(updated_npcs),
      world->objects
    );
    
    return datamodel::world::World::storage::make_entity(std::move(updated_world));
  }

} // namespace history_game::systems::memory

#endif // HISTORY_GAME_SYSTEMS_MEMORY_MEMORY_SYSTEM_H