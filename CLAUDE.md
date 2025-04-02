# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project: History Game

A simulation game focusing on emergent culture, behavior, and history in a speechless world where NPCs develop traditions through observation and memory.

## Build & Test Commands
- Build: `cmake -B build && cmake --build build`
- Run all tests: `cd build && ctest`
- Run single test: `cd build && ./tests/test_name`
- Clean build: `rm -rf build`

## Code Style Guidelines
- **Language**: C++20
- **Build System**: CMake with FetchContent for dependencies
- **Dependencies**:
  - googletest for unit testing
  - [inside-out-objects](https://github.com/ruoso/poc-inside-out-objects) for object lifecycle
- **Naming**:
  - Structs: PascalCase (e.g., MemoryEpisode, ActionSequence)
  - Functions/Methods: camelCase
  - Variables: snake_case
  - Constants: UPPER_SNAKE_CASE
- **Formatting**: 2-space indentation, 100 char line limit
- **Error Handling**: Use exceptions for exceptional cases, return values for expected failures
- **Architecture**: 
  - Always use composition instead of inheritance
  - All data objects will be immutable
  - Always use strong types with variants instead of enums
  - Use component-based design with clear separation of concerns
  - Use cpioo::managed_entity::storage for entity storage and memory management
  - Use concepts whenever you define an abstract concept that will be a template parameter

## Documentation
- Comment classes and public methods using Doxygen style
- Document simulation components with examples
- Follow simulation pillars: speechless world, emergent memory, player as participant