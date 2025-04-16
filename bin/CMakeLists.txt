# Main executable
add_executable(history_game 
 src/history_game/bin/main.cpp
)
target_link_libraries(history_game history_game_datamodel history_game_systems)

