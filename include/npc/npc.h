#ifndef HISTORY_GAME_NPC_H
#define HISTORY_GAME_NPC_H

#include <vector>
#include <string>
#include <cpioo/managed_entity.hpp>
#include "entity/entity.h"
#include "npc/drive.h"
#include "npc/npc_identity.h"
#include "memory/perception_buffer.h"
#include "memory/memory_episode.h"
#include "memory/witnessed_sequence.h"
#include "relationship/relationship.h"

namespace history_game {

/**
 * NPC struct representing a non-player character (or player)
 * All data is immutable
 */
struct NPC {
  // Reference to the NPC's identity (used in memories, preventing cycles)
  const NPCIdentity::ref_type identity;
  
  // The NPC's current drives
  const std::vector<Drive> drives;
  
  // Reference to perception buffer
  const PerceptionBuffer::ref_type perception;
  
  // Episodic memory - sequences that had emotional impact
  const std::vector<MemoryEpisode::ref_type> episodic_memory;
  
  // Observed behaviors
  const std::vector<WitnessedSequence::ref_type> observed_behaviors;
  
  // Relationships with other NPCs (asymmetric)
  const std::vector<Relationship::ref_type> relationships;
  
  // Constructor
  NPC(
    const NPCIdentity::ref_type& npc_identity,
    std::vector<Drive> npc_drives,
    const PerceptionBuffer::ref_type& perception_buffer,
    std::vector<MemoryEpisode::ref_type> episodes,
    std::vector<WitnessedSequence::ref_type> behaviors,
    std::vector<Relationship::ref_type> npc_relationships
  ) : identity(npc_identity),
      drives(std::move(npc_drives)),
      perception(perception_buffer),
      episodic_memory(std::move(episodes)),
      observed_behaviors(std::move(behaviors)),
      relationships(std::move(npc_relationships)) {}
      
  // Define storage type for NPCs
  using storage = cpioo::managed_entity::storage<NPC, 10, uint32_t>;
  using ref_type = storage::ref_type;
};

} // namespace history_game

#endif // HISTORY_GAME_NPC_H