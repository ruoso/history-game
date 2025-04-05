# System Architecture

## Entity Relationship Diagram

The diagram below shows the key entities in the history-game system and their relationships:

```mermaid
graph TD
    %% Core Entities
    Entity["Entity
    ---
    id: string
    position: Position"]
    
    %% NPC Components
    NPC["NPC
    ---
    identity: NPCIdentity::ref_type
    drives: vector<Drive>
    perception: PerceptionBuffer::ref_type
    episodic_memory: vector<MemoryEpisode::ref_type>
    observed_behaviors: vector<WitnessedSequence::ref_type>
    relationships: vector<Relationship::ref_type>"]
    
    NPCIdentity["NPCIdentity
    ---
    entity: Entity::ref_type
    current_action: optional<ActionType>
    target_entity: optional<Entity::ref_type>
    target_object: optional<WorldObject::ref_type>"]
    
    %% Memory System
    PerceptionBuffer["PerceptionBuffer
    ---
    recent_perceptions: vector<MemoryEntry::ref_type>"]
    
    MemoryEntry["MemoryEntry
    ---
    timestamp: uint64_t
    actor: NPCIdentity::ref_type
    action: ActionType
    target_entity: optional<Entity::ref_type>
    target_object: optional<WorldObject::ref_type>"]
    
    MemoryEpisode["MemoryEpisode
    ---
    start_time: uint64_t
    end_time: uint64_t
    action_sequence: ActionSequence::ref_type
    drive_impacts: vector<Drive>
    repetition_count: uint32_t"]
    
    ActionSequence["ActionSequence
    ---
    id: string
    steps: vector<ActionStep>"]
    
    ActionStep["ActionStep
    ---
    memory: MemoryEntry::ref_type
    delay_after_previous: uint32_t"]
    
    WitnessedSequence["WitnessedSequence
    ---
    sequence: ActionSequence::ref_type
    first_witnessed: uint64_t
    times_witnessed: uint32_t"]
    
    %% Drive System
    Drive["Drive
    ---
    type: DriveType
    intensity: float"]
    
    DriveType["DriveType
    ---
    variant<Belonging, Grief, 
            Curiosity, Sustenance,
            Shelter, Pride>"]
    
    %% Actions
    ActionType["ActionType
    ---
    variant<Move, Observe, Give,
            Take, Rest, Build, Plant,
            Bury, Gesture, Follow>"]
    
    %% Relationships
    Relationship["Relationship
    ---
    target: RelationshipTarget
    familiarity: float
    affective_traces: vector<AffectiveTrace>
    last_interaction: uint64_t
    interaction_count: uint32_t"]
    
    RelationshipTarget["RelationshipTarget
    ---
    variant<Entity::ref_type,
            WorldObject::ref_type,
            LocationPoint>"]
    
    AffectiveTrace["AffectiveTrace
    ---
    drive_type: DriveType
    value: float"]
    
    %% World Object
    WorldObject["WorldObject
    ---
    entity: Entity::ref_type
    category: ObjectCategory
    created_by: NPCIdentity::ref_type"]
    
    ObjectCategory["ObjectCategory
    ---
    variant<Food, Structure>"]
    
    %% World
    World["World
    ---
    clock: SimulationClock::ref_type
    npcs: vector<NPC::ref_type>
    objects: vector<WorldObject::ref_type>"]
    
    SimulationClock["SimulationClock
    ---
    current_tick: uint64_t
    current_generation: uint32_t
    ticks_per_generation: uint64_t"]
    
    %% Relationships
    Entity -- "referenced by" --> NPC
    Entity -- "referenced by" --> WorldObject
    
    NPCIdentity -- "identity of" --> NPC
    NPCIdentity -- "references" --> Entity
    
    NPC -- "has perception" --> PerceptionBuffer
    NPC -- "has memories" --> MemoryEpisode
    NPC -- "has behaviors" --> WitnessedSequence
    NPC -- "has relationships" --> Relationship
    NPC -- "has drives" --> Drive
    
    PerceptionBuffer -- "contains" --> MemoryEntry
    MemoryEntry -- "references" --> NPCIdentity
    MemoryEntry -- "has action" --> ActionType
    MemoryEntry -- "may target" --> Entity
    MemoryEntry -- "may target" --> WorldObject
    
    MemoryEpisode -- "contains" --> ActionSequence
    MemoryEpisode -- "has impacts" --> Drive
    
    ActionSequence -- "contains" --> ActionStep
    ActionStep -- "references" --> MemoryEntry
    
    Drive -- "has type" --> DriveType
    
    Relationship -- "has target" --> RelationshipTarget
    Relationship -- "has traces" --> AffectiveTrace
    AffectiveTrace -- "for drive" --> DriveType
    
    RelationshipTarget -- "may be" --> Entity
    RelationshipTarget -- "may be" --> WorldObject
    
    WorldObject -- "references" --> Entity
    WorldObject -- "has category" --> ObjectCategory
    WorldObject -- "created by" --> NPCIdentity
    
    World -- "has clock" --> SimulationClock
    World -- "contains" --> NPC
    World -- "contains" --> WorldObject
```

## Key Systems

The history-game consists of several interconnected systems:

1. **Entity System**: The foundation for all game objects with identity and position.

2. **NPC System**: Representing the characters that inhabit the world, with drives, memories, and relationships.

3. **Memory System**: Handles perception, memory formation, and episodic memory creation.

4. **Drive System**: Manages the emotional drives that motivate NPC behavior.

5. **Relationship System**: Tracks how NPCs relate to other entities, objects, and locations.

6. **Action System**: Defines the possible actions NPCs can take and their impacts.

7. **World System**: Manages the overall simulation state, including time progression.

## Design Principles

1. **Immutable Data**: All entities are immutable. Updates create new instances rather than modifying existing ones.

2. **Component-Based Design**: Clear separation of concerns with well-defined component boundaries.

3. **Strong Typing**: Uses strong types with variants instead of enums for type safety.

4. **Memory Safety**: Managed entity pattern for reference counting and memory management.