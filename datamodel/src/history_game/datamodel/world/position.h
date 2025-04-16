#ifndef HISTORY_GAME_DATAMODEL_WORLD_POSITION_H
#define HISTORY_GAME_DATAMODEL_WORLD_POSITION_H

namespace history_game::datamodel::world {

/**
 * Position struct for spatial coordinates
 * Immutable data structure
 */
struct Position {
  const float x;
  const float y;
  
  // Constructor
  Position(float x_pos, float y_pos) : x(x_pos), y(y_pos) {}
};

} // namespace history_game::datamodel::world

#endif // HISTORY_GAME_DATAMODEL_WORLD_POSITION_H