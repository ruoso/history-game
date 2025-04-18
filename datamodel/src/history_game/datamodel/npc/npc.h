#ifndef HISTORY_GAME_DATAMODEL_NPC_NPC_H
#define HISTORY_GAME_DATAMODEL_NPC_NPC_H

#include <vector>
#include <string>
#include <cpioo/managed_entity.hpp>
#include <history_game/datamodel/entity/entity.h>
#include <history_game/datamodel/npc/drive.h>
#include <history_game/datamodel/npc/npc_identity.h>
#include <history_game/datamodel/memory/perception_buffer.h>
#include <history_game/datamodel/memory/memory_episode.h>
#include <history_game/datamodel/memory/witnessed_sequence.h>
#include <history_game/datamodel/relationship/relationship.h>
#include <history_game/datamodel/relationship/relationship_target.h>

namespace history_game::datamodel::npc {

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
  const memory::PerceptionBuffer::ref_type perception;
  
  // Episodic memory - sequences that had emotional impact
  const std::vector<memory::MemoryEpisode::ref_type> episodic_memory;
  
  // Observed behaviors
  const std::vector<memory::WitnessedSequence::ref_type> observed_behaviors;
  
  // Relationships with other NPCs (asymmetric)
  const std::vector<relationship::Relationship::ref_type> relationships;
  
  // Constructor
  NPC(
    const NPCIdentity::ref_type& npc_identity,
    std::vector<Drive> npc_drives,
    const memory::PerceptionBuffer::ref_type& perception_buffer,
    std::vector<memory::MemoryEpisode::ref_type> episodes,
    std::vector<memory::WitnessedSequence::ref_type> behaviors,
    std::vector<relationship::Relationship::ref_type> npc_relationships
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

} // namespace history_game::datamodel::npc

#endif // HISTORY_GAME_DATAMODEL_NPC_NPC_H