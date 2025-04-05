# History Game Simulation Visualizer

This is a web-based visualization tool for the History Game simulation. It allows you to load and replay simulation logs, observe NPC behaviors, actions, and interactions in a graphical format.

## Features

- Playback simulation events with variable speed control
- Visualize NPCs, objects, and their positions in the world
- Highlight perceptions, actions, and interactions between entities
- View NPC drives and their changes over time
- Inspect detailed information about selected entities
- Track event logs during simulation replay

## How to Use

1. **Open the Visualizer**: 
   - Open the `index.html` file in a web browser
   - No server required - it runs entirely client-side

2. **Load a Simulation Log**:
   - Click "Browse" to select a JSON log file
   - Click "Load Simulation" to load the data

3. **Playback Controls**:
   - Play/Pause: Start or pause the simulation playback
   - Step Forward/Backward: Move one tick at a time
   - Speed Control: Adjust playback speed from 0.1x to 10x
   - Timeline Slider: Jump to a specific point in the simulation

4. **Visualization Interaction**:
   - Click on entities to view detailed information
   - Observe the event log for a history of actions
   - Monitor simulation statistics and current tick information

## Log File Format

The visualizer expects a JSON array of events with the following structure:

```json
[
  {
    "timestamp": 1712345678901,
    "type": "SIMULATION_START",
    "npc_count": 100,
    "object_count": 100
  },
  {
    "timestamp": 1712345678910,
    "type": "TICK_START",
    "tick_number": 1,
    "generation": 1
  },
  {
    "timestamp": 1712345678920,
    "type": "ENTITY_UPDATE",
    "entity_id": "npc_1234",
    "entity_type": "NPC",
    "position": {"x": 123.4, "y": 456.7},
    "drives": [
      {"type": "Belonging", "value": 0.7},
      {"type": "Curiosity", "value": 0.3}
    ],
    "current_action": "Observe"
  },
  {
    "timestamp": 1712345678930,
    "type": "ACTION_EXECUTION",
    "entity_id": "npc_1234",
    "action_type": "Observe",
    "target_id": "npc_5678"
  },
  // ... more events
]
```

## Supported Event Types

1. `SIMULATION_START`: Initial simulation parameters
2. `SIMULATION_END`: Final statistics and results
3. `TICK_START`: Beginning of a simulation tick
4. `TICK_END`: End of a simulation tick
5. `ENTITY_UPDATE`: Position and state updates for entities
6. `ACTION_EXECUTION`: Actions performed by entities

## Development

This visualizer is built with:
- HTML5
- CSS3
- Vanilla JavaScript (no external dependencies)
- HTML5 Canvas for rendering

## Future Enhancements

Possible future improvements:
- Heatmap visualization of activity
- Graph visualization of social relationships
- Timeline charts of drive changes
- Filtering and search capabilities
- Export of visualizations as images or videos