#ifndef HISTORY_GAME_PERCEPTION_SYSTEM_H
#define HISTORY_GAME_PERCEPTION_SYSTEM_H

#include <vector>
#include <tuple>
#include <cmath>
#include <variant>
#include "world/world.h"
#include "npc/npc.h"
#include "object/object.h"

namespace history_game {

namespace perception_system {

  /**
   * Represents an entity that can be perceived (NPC or WorldObject)
   * using a variant to hold either type
   */
  using PerceivableEntity = std::variant<NPC::ref_type, WorldObject::ref_type>;

  /**
   * Represents a pair of entities and their distance from each other
   */
  struct PerceptionPair {
    // The perceiving NPC
    const NPC::ref_type perceiver;
    
    // The entity being perceived (NPC or WorldObject)
    const PerceivableEntity perceived;
    
    // Distance between them
    const float distance;
    
    // Constructor
    PerceptionPair(
      const NPC::ref_type& npc,
      const PerceivableEntity& entity,
      float dist
    ) : perceiver(npc),
        perceived(entity),
        distance(dist) {}
  };

  /**
   * Position getter function overloads for argument-dependent lookup
   */
  inline const Position& getPosition(const NPC::ref_type& npc) {
    return npc->identity->entity->position;
  }

  inline const Position& getPosition(const WorldObject::ref_type& obj) {
    return obj->entity->position;
  }

  /**
   * ID getter function overloads for argument-dependent lookup
   */
  inline const std::string& getId(const NPC::ref_type& npc) {
    return npc->identity->entity->id;
  }

  inline const std::string& getId(const WorldObject::ref_type& obj) {
    return obj->entity->id;
  }

  /**
   * Calculate distance between two positions
   */
  inline float calculateDistance(const Position& pos1, const Position& pos2) {
    float dx = pos1.x - pos2.x;
    float dy = pos1.y - pos2.y;
    return std::sqrt(dx*dx + dy*dy);
  }
  
  /**
   * Get position of any perceivable entity using std::visit with a lambda
   */
  inline const Position& getEntityPosition(const PerceivableEntity& entity) {
    return std::visit([](const auto& e) -> const Position& { return getPosition(e); }, entity);
  }
  
  /**
   * Get ID of any perceivable entity using std::visit with a lambda
   */
  inline const std::string& getEntityId(const PerceivableEntity& entity) {
    return std::visit([](const auto& e) -> const std::string& { return getId(e); }, entity);
  }
  
  /**
   * Find all entity pairs within perception range of each other
   * 
   * @param world The current world state
   * @param max_distance Maximum distance for perception
   * @return Vector of all perceptible entity pairs with their distances
   */
  inline std::vector<PerceptionPair> calculatePerceptibleEntities(
    const World::ref_type& world,
    float max_distance = 10.0f
  ) {
    std::vector<PerceptionPair> result;
    
    // For each NPC...
    for (const auto& npc : world->npcs) {
      const Position& npc_pos = getPosition(npc);
      
      // Check other NPCs
      for (const auto& other_npc : world->npcs) {
        // Skip self
        if (getId(npc) == getId(other_npc)) {
          continue;
        }
        
        const Position& other_pos = getPosition(other_npc);
        float distance = calculateDistance(npc_pos, other_pos);
        
        if (distance <= max_distance) {
          result.emplace_back(npc, PerceivableEntity{other_npc}, distance);
        }
      }
      
      // Check world objects
      for (const auto& object : world->objects) {
        const Position& obj_pos = getPosition(object);
        float distance = calculateDistance(npc_pos, obj_pos);
        
        if (distance <= max_distance) {
          result.emplace_back(npc, PerceivableEntity{object}, distance);
        }
      }
    }
    
    return result;
  }

} // namespace perception_system

} // namespace history_game

#endif // HISTORY_GAME_PERCEPTION_SYSTEM_H