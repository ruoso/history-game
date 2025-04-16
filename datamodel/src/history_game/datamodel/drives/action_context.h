#ifndef HISTORY_GAME_DATAMODEL_DRIVES_ACTION_CONTEXT_H
#define HISTORY_GAME_DATAMODEL_DRIVES_ACTION_CONTEXT_H

#include <vector>
#include <string>
#include <history_game/datamodel/npc/npc.h>
#include <history_game/datamodel/memory/memory_entry.h>

namespace history_game::datamodel::drives {

/**
 * Context information for evaluating drive impacts based on the spec
 */
struct ActionContext {
  // The NPC experiencing/evaluating the action
  const npc::NPC::ref_type observer;
  
  // The memory entry being evaluated
  const memory::MemoryEntry::ref_type memory;
  
  // Current simulation time
  const uint64_t current_time;
  
  // Constructor
  ActionContext(
    const npc::NPC::ref_type& npc,
    const memory::MemoryEntry::ref_type& memory_entry,
    uint64_t time
  ) : observer(npc),
      memory(memory_entry),
      current_time(time) {}
};

} // namespace history_game::datamodel::drives

#endif // HISTORY_GAME_DATAMODEL_DRIVES_ACTION_CONTEXT_H