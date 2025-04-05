# History Game

A simulation game focusing on emergent culture, behavior, and history in a speechless world where NPCs develop traditions through observation and memory.

## Overview

This is an emergent simulation where history, culture, and traditions evolve organically through the actions and memories of the inhabitants. The game world has no scripted narratives or authored content - every ritual, tradition, and cultural behavior emerges from the interactions between NPCs over multiple generations.

## Core Simulation Pillars

- **Speechless world**: All communication is behavioral and emotional
- **Emergent memory and culture**: No hardcoded lore or events
- **Player is one of many**: The player acts as a participant, not a director
- **History is lived**: Every environmental trace is the result of actual past behaviors

## Project Structure

This C++20 project uses:
- CMake for build configuration
- [inside-out-objects](https://github.com/ruoso/poc-inside-out-objects) for entity lifecycle management
- GoogleTest for testing
- Immutable data structures for simulation state

See the [System Architecture](docs/system_architecture.md) for a detailed overview of the components and their relationships.

## Building

```bash
cmake -B build
cmake --build build
```

## Running Tests

```bash
cd build && ctest
```

## Design Principles

- All data structures are immutable
- Composition over inheritance
- Strong typing with variants instead of enums
- NPCIdentity pattern to prevent circular references
- Drive-based action selection and emotional state management

## Optimization Features

The simulation is designed to efficiently handle large numbers of NPCs and objects:

- **Spatial Partitioning**: The perception system uses a grid-based spatial partitioning algorithm to reduce complexity from O(n²) to closer to O(n).
- **Efficient Memory Management**: The inside-out-objects library provides reference counting and memory reuse.
- **Component-Based Architecture**: Clear separation of systems allows for targeted optimizations.
- **Immutable Data**: All data structures are immutable, allowing for lockless parallelism in future implementations.

## Status

This project is in early development. The core data structures and architecture are in place, but additional simulation features and rendering are still being implemented.