#ifndef HISTORY_GAME_WITNESSED_SEQUENCE_H
#define HISTORY_GAME_WITNESSED_SEQUENCE_H

#include <cstdint>
#include <vector>
#include <cpioo/managed_entity.hpp>
#include <history_game/datamodel/action/action_sequence.h>
#include <history_game/datamodel/npc/npc_identity.h>
#include <history_game/datamodel/npc/drive.h>

namespace history_game {

/**
 * Perceived effectiveness of a sequence for a specific drive
 */
struct PerceivedEffectiveness {
  const DriveType drive_type;
  const float value;
  
  // Constructor
  template<DriveTypeConcept T>
  PerceivedEffectiveness(T type, float effectiveness)
    : drive_type(type), value(effectiveness) {}
};

/**
 * Represents a behavior sequence that has been observed
 * and might be imitated
 */
struct WitnessedSequence {
  // The action sequence that was observed
  const ActionSequence::ref_type sequence;
  
  // The NPC who performed the sequence
  const NPCIdentity::ref_type performer;
  
  // Number of times witnessed
  const uint32_t observation_count;
  
  // Perceived effectiveness per drive (subjective to the observer)
  const std::vector<PerceivedEffectiveness> effectiveness;
  
  // Constructor
  WitnessedSequence(
    const ActionSequence::ref_type& action_sequence,
    const NPCIdentity::ref_type& sequence_performer,
    uint32_t times_witnessed,
    std::vector<PerceivedEffectiveness> drive_effectiveness
  ) : sequence(action_sequence),
      performer(sequence_performer),
      observation_count(times_witnessed),
      effectiveness(std::move(drive_effectiveness)) {}
      
  // Define storage type
  using storage = cpioo::managed_entity::storage<WitnessedSequence, 10, uint32_t>;
  using ref_type = storage::ref_type;
};

} // namespace history_game

#endif // HISTORY_GAME_WITNESSED_SEQUENCE_H