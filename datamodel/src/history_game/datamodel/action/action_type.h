#ifndef HISTORY_GAME_ACTION_TYPE_H
#define HISTORY_GAME_ACTION_TYPE_H

#include <string>
#include <string_view>
#include <variant>
#include <concepts>

namespace history_game {

/**
 * Strong types for different action types
 * Based on the actions specified in the spec:
 * Move, Observe, Give/Take, Rest, Build, Plant, Bury, Gesture, Follow
 */
namespace action_type {
  struct Move {
    static constexpr auto name = "Move";
  };
  
  struct Observe {
    static constexpr auto name = "Observe";
  };
  
  struct Give {
    static constexpr auto name = "Give";
  };
  
  struct Take {
    static constexpr auto name = "Take";
  };
  
  struct Rest {
    static constexpr auto name = "Rest";
  };
  
  struct Build {
    static constexpr auto name = "Build";
  };
  
  struct Plant {
    static constexpr auto name = "Plant";
  };
  
  struct Bury {
    static constexpr auto name = "Bury";
  };
  
  struct Gesture {
    static constexpr auto name = "Gesture";
  };
  
  struct Follow {
    static constexpr auto name = "Follow";
  };
}

/**
 * Concept for action types that ensures they have a name
 */
template<typename T>
concept ActionTypeConcept = requires {
  { T::name } -> std::convertible_to<std::string_view>;
};

/**
 * Variant to represent any action type
 */
using ActionType = std::variant<
  action_type::Move,
  action_type::Observe,
  action_type::Give,
  action_type::Take,
  action_type::Rest,
  action_type::Build,
  action_type::Plant,
  action_type::Bury,
  action_type::Gesture,
  action_type::Follow
>;

} // namespace history_game

#endif // HISTORY_GAME_ACTION_TYPE_H