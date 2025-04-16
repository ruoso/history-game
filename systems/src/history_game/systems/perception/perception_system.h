#ifndef HISTORY_GAME_PERCEPTION_SYSTEM_H
#define HISTORY_GAME_PERCEPTION_SYSTEM_H

#include <vector>
#include <tuple>
#include <cmath>
#include <variant>
#include <unordered_map>
#include <algorithm>
#include <spdlog/spdlog.h>
#include <history_game/datamodel/world/world.h>
#include <history_game/datamodel/npc/npc.h>
#include <history_game/datamodel/object/object.h>

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
   * Simple spatial cell for partitioning
   */
  struct SpatialCell {
    std::vector<NPC::ref_type> npcs;
    std::vector<WorldObject::ref_type> objects;
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
   * Get entity type name for NPC
   */
  inline std::string get_entity_type_name(const NPC::ref_type& npc) {
    return "NPC";
  }
  
  /**
   * Get entity type name for WorldObject
   */
  inline std::string get_entity_type_name(const WorldObject::ref_type& obj) {
    return std::visit([](const auto& category) -> std::string { 
      return category.name; 
    }, obj->category);
  }
  
  /**
   * Get entity type name using std::visit
   */
  inline std::string get_entity_type_name(const PerceivableEntity& entity) {
    return std::visit([](const auto& e) -> std::string {
      return get_entity_type_name(e);
    }, entity);
  }
  
  /**
   * Get entity ID for logging
   */
  inline std::string get_entity_id(const NPC::ref_type& npc) {
    return npc->identity->entity->id;
  }
  
  /**
   * Get entity ID for WorldObject
   */
  inline std::string get_entity_id(const WorldObject::ref_type& obj) {
    return obj->entity->id;
  }
  
  /**
   * Get entity ID using std::visit
   */
  inline std::string get_entity_id(const PerceivableEntity& entity) {
    return std::visit([](const auto& e) -> std::string {
      return get_entity_id(e);
    }, entity);
  }
  
  /**
   * Calculate cell index for a position
   */
  inline std::pair<int, int> getCellIndices(const Position& pos, float cell_size) {
    int x_idx = static_cast<int>(pos.x / cell_size);
    int y_idx = static_cast<int>(pos.y / cell_size);
    return {x_idx, y_idx};
  }

  /**
   * Create cell key from indices
   */
  inline int64_t getCellKey(int x_idx, int y_idx) {
    // Combine x and y indices into a single key
    return (static_cast<int64_t>(x_idx) << 32) | static_cast<uint32_t>(y_idx);
  }

  /**
   * Find all entity pairs within perception range of each other
   * using spatial partitioning for improved efficiency
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
    
    // Choose cell size based on perception distance
    float cell_size = max_distance;
    
    // Create spatial grid
    std::unordered_map<int64_t, SpatialCell> grid;
    
    // Populate the grid with NPCs
    for (const auto& npc : world->npcs) {
      const Position& pos = getPosition(npc);
      auto [x_idx, y_idx] = getCellIndices(pos, cell_size);
      grid[getCellKey(x_idx, y_idx)].npcs.push_back(npc);
    }
    
    // Populate the grid with objects
    for (const auto& obj : world->objects) {
      const Position& pos = getPosition(obj);
      auto [x_idx, y_idx] = getCellIndices(pos, cell_size);
      grid[getCellKey(x_idx, y_idx)].objects.push_back(obj);
    }
    
    // For each NPC, check nearby cells for perceptible entities
    for (const auto& npc : world->npcs) {
      const Position& npc_pos = getPosition(npc);
      auto [center_x, center_y] = getCellIndices(npc_pos, cell_size);
      
      // Check cells in the 3x3 grid centered on the NPC's cell
      for (int x_offset = -1; x_offset <= 1; ++x_offset) {
        for (int y_offset = -1; y_offset <= 1; ++y_offset) {
          int x_idx = center_x + x_offset;
          int y_idx = center_y + y_offset;
          int64_t cell_key = getCellKey(x_idx, y_idx);
          
          // Skip if the cell doesn't exist
          if (grid.find(cell_key) == grid.end()) {
            continue;
          }
          
          const SpatialCell& cell = grid[cell_key];
          
          // Check other NPCs in this cell
          for (const auto& other_npc : cell.npcs) {
            // Skip self
            if (getId(npc) == getId(other_npc)) {
              continue;
            }
            
            const Position& other_pos = getPosition(other_npc);
            float distance = calculateDistance(npc_pos, other_pos);
            
            if (distance <= max_distance) {
              // Log the perception
              const std::string observer_id = get_entity_id(npc);
              const std::string observed_id = get_entity_id(other_npc);
              spdlog::debug("NPC {} perceives NPC {} at distance {:.2f}", 
                          observer_id, observed_id, distance);
                        
              result.emplace_back(npc, PerceivableEntity{other_npc}, distance);
            }
          }
          
          // Check objects in this cell
          for (const auto& object : cell.objects) {
            const Position& obj_pos = getPosition(object);
            float distance = calculateDistance(npc_pos, obj_pos);
            
            if (distance <= max_distance) {
              // Log the perception
              const std::string observer_id = get_entity_id(npc);
              const std::string object_id = get_entity_id(object);
              const std::string object_type = get_entity_type_name(object);
              spdlog::debug("NPC {} perceives {} {} at distance {:.2f}", 
                          observer_id, object_type, object_id, distance);
                        
              result.emplace_back(npc, PerceivableEntity{object}, distance);
            }
          }
        }
      }
    }
    
    return result;
  }

} // namespace perception_system

} // namespace history_game

#endif // HISTORY_GAME_PERCEPTION_SYSTEM_H