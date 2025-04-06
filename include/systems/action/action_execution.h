#ifndef HISTORY_GAME_ACTION_EXECUTION_H
#define HISTORY_GAME_ACTION_EXECUTION_H

#include <random>
#include <spdlog/spdlog.h>
#include "datamodel/world/world.h"
#include "datamodel/action/action_type.h"
#include "datamodel/npc/npc.h"
#include "datamodel/object/object.h"
#include "systems/behavior/action_selection.h"
#include "systems/utility/serialization.h"

namespace history_game {
namespace action_execution_system {

/**
 * Visitor for executing different action types
 */
struct ActionExecutor {
    const World::ref_type& world;
    const NPC::ref_type& npc;
    
    // Random generators for stochastic behavior
    static inline std::random_device rd{};
    static inline std::mt19937 gen{rd()};
    
    // Helper method to update NPC entity position
    NPC::ref_type updateNPCPosition(const Position& new_position) const {
        auto npc_identity = npc->identity;
        auto entity = npc_identity->entity;
        
        // Create new entity with updated position
        Entity new_entity(entity->id, new_position);
        auto new_entity_ref = Entity::storage::make_entity(std::move(new_entity));
        
        // Create new NPCIdentity with updated entity based on current action and targets
        NPCIdentity::ref_type new_identity_ref = [&]() {
            if (npc_identity->current_action) {
                if (npc_identity->target_entity) {
                    // Action with entity target
                    NPCIdentity new_identity(
                        new_entity_ref,
                        npc_identity->current_action.value(),
                        npc_identity->target_entity.value()
                    );
                    return NPCIdentity::storage::make_entity(std::move(new_identity));
                } 
                else if (npc_identity->target_object) {
                    // Action with object target
                    NPCIdentity new_identity(
                        new_entity_ref,
                        npc_identity->current_action.value(),
                        npc_identity->target_object.value()
                    );
                    return NPCIdentity::storage::make_entity(std::move(new_identity));
                } 
                else {
                    // Action with no target
                    NPCIdentity new_identity(
                        new_entity_ref,
                        npc_identity->current_action.value()
                    );
                    return NPCIdentity::storage::make_entity(std::move(new_identity));
                }
            } 
            else {
                // No action
                NPCIdentity new_identity(new_entity_ref);
                return NPCIdentity::storage::make_entity(std::move(new_identity));
            }
        }();
        
        // Create new NPC with updated identity
        NPC updated_npc(
            new_identity_ref, 
            npc->drives, 
            npc->perception,
            npc->episodic_memory,
            npc->observed_behaviors,  // Correct field name (was known_entities)
            npc->relationships
        );
        return NPC::storage::make_entity(std::move(updated_npc));
    }
    
    // Move action handler
    NPC::ref_type operator()(const action_type::Move&) const {
        auto npc_identity = npc->identity;
        auto position = npc_identity->entity->position;
        
        // If we have a target, move toward it
        if (npc_identity->target_entity) {
            auto target_pos = npc_identity->target_entity.value()->position;
            float dx = target_pos.x - position.x;
            float dy = target_pos.y - position.y;
            
            // Calculate distance
            float distance = std::sqrt(dx*dx + dy*dy);
            
            // If we're already close, don't move
            if (distance < 10.0f) {
                return npc;
            }
            
            // Calculate normalized direction
            float move_speed = 30.0f; // Units per tick
            float normalized_dx = dx / distance;
            float normalized_dy = dy / distance;
            
            // Update position (move up to max speed or remaining distance)
            float actual_move = std::min(move_speed, distance);
            float new_x = position.x + normalized_dx * actual_move;
            float new_y = position.y + normalized_dy * actual_move;
            
            Position new_position(new_x, new_y);
            
            // Log movement in the debug output
            spdlog::debug("NPC {} moved from ({:.1f}, {:.1f}) to ({:.1f}, {:.1f})",
                         npc->identity->entity->id,
                         position.x, position.y,
                         new_position.x, new_position.y);
            
            return updateNPCPosition(new_position);
        }
        else {
            // Random movement if no target
            std::uniform_real_distribution<> dir_dist(-1.0, 1.0);
            std::uniform_real_distribution<> speed_dist(5.0, 20.0);
            
            float dx = dir_dist(gen);
            float dy = dir_dist(gen);
            float move_speed = speed_dist(gen);
            
            // Normalize and scale movement
            float length = std::sqrt(dx*dx + dy*dy);
            if (length > 0) {
                dx /= length;
                dy /= length;
            }
            
            float new_x = position.x + dx * move_speed;
            float new_y = position.y + dy * move_speed;
            
            // Bounds checking (assuming world size of 1000x1000)
            const float WORLD_SIZE = 1000.0f;
            new_x = std::max(0.0f, std::min(WORLD_SIZE, new_x));
            new_y = std::max(0.0f, std::min(WORLD_SIZE, new_y));
            
            Position new_position(new_x, new_y);
            
            // Log movement in the debug output
            spdlog::debug("NPC {} moved randomly from ({:.1f}, {:.1f}) to ({:.1f}, {:.1f})",
                         npc->identity->entity->id,
                         position.x, position.y,
                         new_position.x, new_position.y);
            
            return updateNPCPosition(new_position);
        }
    }
    
    // Observe action handler
    NPC::ref_type operator()(const action_type::Observe&) const {
        // Observe action doesn't change the NPC directly
        // The perception system will handle adding observations to memory
        return npc;
    }
    
    // Take action handler
    NPC::ref_type operator()(const action_type::Take&) const {
        // Take action: Move an object closer to the NPC
        if (npc->identity->target_object) {
            // In a real implementation, we might:
            // 1. Change the object's position to be near the NPC
            // 2. Update the object's ownership
            // 3. Add the object to the NPC's inventory
            // For now, we'll just leave this as a placeholder
        }
        return npc;
    }
    
    // Give action handler
    NPC::ref_type operator()(const action_type::Give&) const {
        // Give action: Transfer an object to another NPC
        if (npc->identity->target_entity && npc->identity->target_object) {
            // In a real implementation, we might:
            // 1. Change the object's position to be near the target NPC
            // 2. Update the object's ownership
            // 3. Add the object to the target NPC's inventory
            // For now, we'll just leave this as a placeholder
        }
        return npc;
    }
    
    // Rest action handler
    NPC::ref_type operator()(const action_type::Rest&) const {
        // Rest action: NPC stays in place and recovers energy
        // For now, just return the NPC unchanged
        return npc;
    }
    
    // Build action handler
    NPC::ref_type operator()(const action_type::Build&) const {
        // Build action: Create a structure at the NPC's location
        // For now, just return the NPC unchanged
        return npc;
    }
    
    // Plant action handler
    NPC::ref_type operator()(const action_type::Plant&) const {
        // Plant action: Create a plant at the NPC's location
        // For now, just return the NPC unchanged
        return npc;
    }
    
    // Bury action handler
    NPC::ref_type operator()(const action_type::Bury&) const {
        // Bury action: Hide an object underground
        // For now, just return the NPC unchanged
        return npc;
    }
    
    // Gesture action handler
    NPC::ref_type operator()(const action_type::Gesture&) const {
        // Gesture action: Make a symbolic gesture toward a target
        // For now, just return the NPC unchanged
        return npc;
    }
    
    // Follow action handler
    NPC::ref_type operator()(const action_type::Follow&) const {
        // Similar to Move but meant to track a target over time
        // For now, handle it the same as Move
        return (*this)(action_type::Move{});
    }
};

/**
 * Execute an NPC's current action
 * 
 * @param world The current world state
 * @param npc_ref Reference to the NPC whose action to execute
 * @return Updated NPC with action results applied
 */
inline NPC::ref_type executeAction(
    const World::ref_type& world, 
    const NPC::ref_type& npc,
    serialization::SimulationLogger* logger = nullptr
) {
    // If NPC has no current action, return the NPC unchanged
    if (!npc->identity->current_action) {
        return npc;
    }

    // Get the action type
    auto action_type = npc->identity->current_action.value();
        
    // Log the action to the serialization logger if provided
    if (logger && logger->isInitialized()) {
        uint64_t current_time = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
        
        // Get target ID if exists
        std::optional<std::string> target_id;
        if (npc->identity->target_entity) {
            target_id = npc->identity->target_entity.value()->id;
        } else if (npc->identity->target_object) {
            target_id = npc->identity->target_object.value()->entity->id;
        }
        
        // Log action execution event
        logger->logEvent(serialization::createActionExecutionEvent(
            current_time,
            npc->identity->entity->id,
            action_selection_system::get_action_name(action_type),
            target_id
        ));
    }
    
    // Use a visitor to execute the appropriate action
    ActionExecutor executor{world, npc};
    return std::visit(executor, action_type);
}

/**
 * Execute all NPC actions in the world
 * 
 * @param world The current world state
 * @param logger Optional serialization logger for event recording
 * @return Updated world with all actions executed
 */
inline World::ref_type executeAllActions(
    const World::ref_type& world,
    serialization::SimulationLogger* logger = nullptr
) {
    spdlog::info("Executing actions for all NPCs at tick {}", world->clock->current_tick);
    
    std::vector<NPC::ref_type> updated_npcs;
    updated_npcs.reserve(world->npcs.size());
    
    // Execute actions for each NPC
    for (const auto& npc : world->npcs) {
        auto updated_npc = executeAction(world, npc, logger);
        updated_npcs.push_back(updated_npc);
    }
    
    // Create a new world with updated NPCs
    World updated_world(world->clock, updated_npcs, world->objects);
    return World::storage::make_entity(std::move(updated_world));
}

} // namespace action_execution_system
} // namespace history_game

#endif // HISTORY_GAME_ACTION_EXECUTION_H