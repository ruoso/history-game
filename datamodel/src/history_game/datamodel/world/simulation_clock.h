#ifndef HISTORY_GAME_DATAMODEL_WORLD_SIMULATION_CLOCK_H
#define HISTORY_GAME_DATAMODEL_WORLD_SIMULATION_CLOCK_H

#include <cstdint>
#include <string>
#include <cpioo/managed_entity.hpp>

namespace history_game {

/**
 * Manages simulation time and generational tracking
 */
struct SimulationClock {
  // Current tick count
  const uint64_t current_tick;
  
  // Current generation number
  const uint32_t current_generation;
  
  // Ticks per generation
  const uint32_t ticks_per_generation;
  
  // Constructor
  SimulationClock(
    uint64_t tick,
    uint32_t generation,
    uint32_t generation_length
  ) : current_tick(tick),
      current_generation(generation),
      ticks_per_generation(generation_length) {}
      
  // Define storage type
  using storage = cpioo::managed_entity::storage<SimulationClock, 4, uint16_t>;
  using ref_type = storage::ref_type;
};

} // namespace history_game

#endif // HISTORY_GAME_DATAMODEL_WORLD_SIMULATION_CLOCK_H