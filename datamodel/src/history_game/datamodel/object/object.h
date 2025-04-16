#ifndef HISTORY_GAME_DATAMODEL_OBJECT_OBJECT_H
#define HISTORY_GAME_DATAMODEL_OBJECT_OBJECT_H

#include <string>
#include <vector>
#include <variant>
#include <concepts>
#include <cpioo/managed_entity.hpp>
#include <history_game/datamodel/entity/entity.h>
#include <history_game/datamodel/npc/npc_identity.h>

namespace history_game::datamodel::object {

/**
 * Strong types for object categories
 */
namespace object_category {
  struct Food {
    static constexpr auto name = "Food";
  };
  
  struct Structure {
    static constexpr auto name = "Structure";
  };
  
  struct Tool {
    static constexpr auto name = "Tool";
  };
  
  struct Burial {
    static constexpr auto name = "Burial";
  };
  
  struct Plant {
    static constexpr auto name = "Plant";
  };
  
  struct Marker {
    static constexpr auto name = "Marker";
  };
}

/**
 * Concept for object category types that ensures they have a name
 */
template<typename T>
concept ObjectCategoryConcept = requires {
  { T::name } -> std::convertible_to<std::string_view>;
};

/**
 * Variant to represent any object category
 */
using ObjectCategory = std::variant<
  object_category::Food,
  object_category::Structure,
  object_category::Tool,
  object_category::Burial,
  object_category::Plant,
  object_category::Marker
>;

/**
 * Represents a world object
 */
struct WorldObject {
  // Reference to the base entity
  const entity::Entity::ref_type entity;
  
  // Object category
  const ObjectCategory category;
  
  // Creator of this object (if any)
  const npc::NPCIdentity::ref_type created_by;
  
  // Constructor
  template<ObjectCategoryConcept T>
  WorldObject(
    const entity::Entity::ref_type& entity_ref,
    T category_type,
    const npc::NPCIdentity::ref_type& creator
  ) : entity(entity_ref),
      category(category_type),
      created_by(creator) {}
      
  // Define storage type
  using storage = cpioo::managed_entity::storage<WorldObject, 10, uint32_t>;
  using ref_type = storage::ref_type;
};

} // namespace history_game::datamodel::object

#endif // HISTORY_GAME_DATAMODEL_OBJECT_OBJECT_H