#ifndef HISTORY_GAME_DRIVE_DYNAMICS_H
#define HISTORY_GAME_DRIVE_DYNAMICS_H

#include <vector>
#include <cmath>
#include "npc/npc.h"
#include "npc/drive.h"

namespace history_game {

/**
 * Parameters controlling how drives change over time
 */
struct DriveParameters {
  // Base rate at which drives increase naturally (per tick)
  const float base_growth_rate;
  
  // How much more quickly high drives grow compared to low ones
  const float intensity_factor;
  
  // Different growth rates for each drive type
  const std::vector<std::pair<DriveType, float>> drive_growth_modifiers;
  
  // Constructor with default values
  DriveParameters(
    float growth_rate = 0.1f,
    float intensity = 0.5f,
    std::vector<std::pair<DriveType, float>> modifiers = {}
  ) : base_growth_rate(growth_rate),
      intensity_factor(intensity),
      drive_growth_modifiers(std::move(modifiers)) {}
};

namespace drive_dynamics_system {

  /**
   * Get the growth modifier for a specific drive type
   */
  inline float getGrowthModifier(
    const DriveType& drive_type,
    const std::vector<std::pair<DriveType, float>>& modifiers
  ) {
    // Default modifier if not found
    float modifier = 1.0f;
    
    // Look for a specific modifier for this drive type
    for (const auto& [type, mod] : modifiers) {
      if (drive_impact_system::areSameDriveTypes(drive_type, type)) {
        modifier = mod;
        break;
      }
    }
    
    return modifier;
  }
  
  /**
   * Update a single drive based on time passing
   */
  inline Drive updateDrive(
    const Drive& drive,
    const DriveParameters& params,
    uint64_t ticks_elapsed
  ) {
    // Get the growth modifier for this drive type
    float growth_modifier = getGrowthModifier(
      drive.type,
      params.drive_growth_modifiers
    );
    
    // Calculate the natural increase rate, adjusted for this drive
    float increase_rate = params.base_growth_rate * growth_modifier;
    
    // Higher intensity drives grow faster (using the intensity factor)
    float intensity_multiplier = 1.0f + ((drive.intensity / 100.0f) * params.intensity_factor);
    
    // Calculate the total increase over the elapsed time
    float increase = increase_rate * intensity_multiplier * static_cast<float>(ticks_elapsed);
    
    // Calculate the new intensity, clamped to 0-100
    float new_intensity = std::min(100.0f, drive.intensity + increase);
    
    // Return updated drive
    return Drive(drive.type, new_intensity);
  }
  
  /**
   * Update all drives for an NPC based on time passing
   */
  inline NPC::ref_type updateDrives(
    const NPC::ref_type& npc,
    const DriveParameters& params,
    uint64_t ticks_elapsed
  ) {
    // Update each drive
    std::vector<Drive> updated_drives;
    updated_drives.reserve(npc->drives.size());
    
    for (const auto& drive : npc->drives) {
      updated_drives.push_back(
        updateDrive(drive, params, ticks_elapsed)
      );
    }
    
    // Create a new NPC with updated drives
    NPC updated_npc(
      npc->identity,
      updated_drives,
      npc->perception,
      npc->episodic_memory,
      npc->observed_behaviors,
      npc->relationships
    );
    
    return NPC::storage::make_entity(std::move(updated_npc));
  }

} // namespace drive_dynamics_system

} // namespace history_game

#endif // HISTORY_GAME_DRIVE_DYNAMICS_H