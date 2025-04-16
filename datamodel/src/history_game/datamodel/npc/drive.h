#ifndef HISTORY_GAME_DATAMODEL_NPC_DRIVE_H
#define HISTORY_GAME_DATAMODEL_NPC_DRIVE_H

#include <string>
#include <variant>
#include <concepts>

namespace history_game {

/**
 * Strong types for each emotional drive
 */
namespace drive {
  struct Belonging {
    static constexpr auto name = "Belonging";
  };
  
  struct Grief {
    static constexpr auto name = "Grief";
  };
  
  struct Curiosity {
    static constexpr auto name = "Curiosity";
  };
  
  struct Sustenance {
    static constexpr auto name = "Sustenance";
  };
  
  struct Shelter {
    static constexpr auto name = "Shelter";
  };
  
  struct Pride {
    static constexpr auto name = "Pride";
  };
}

/**
 * Concept for drive types that ensures they have a name
 */
template<typename T>
concept DriveTypeConcept = requires {
  { T::name } -> std::convertible_to<std::string_view>;
};

/**
 * Variant to represent any drive type
 */
using DriveType = std::variant<
  drive::Belonging,
  drive::Grief,
  drive::Curiosity,
  drive::Sustenance,
  drive::Shelter,
  drive::Pride
>;

/**
 * Structure to represent a drive with its intensity
 */
struct Drive {
  const DriveType type;
  const float intensity;
  
  // Constructor with concept constraint
  template<DriveTypeConcept T>
  Drive(T drive_type, float drive_intensity) 
    : type(drive_type), intensity(drive_intensity) {}
    
  // Constructor taking DriveType directly
  Drive(const DriveType& drive_type, float drive_intensity)
    : type(drive_type), intensity(drive_intensity) {}
};

} // namespace history_game

#endif // HISTORY_GAME_DATAMODEL_NPC_DRIVE_H