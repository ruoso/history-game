# Game Design Spec

## Title (WIP)
*A living world where history is participative and emergent.*

---

## Emotional Core

> The player should feel like a participant in an ancient, ongoing world shaped by many lives before and around them — where they discover their connection to the past by encountering the traces left by generations before them.

The game world is speechless. There is no written or spoken language. All meaning, memory, and cultural behavior emerge from action, emotion, and observation.

---

## MVP Scope

### World
- A small, isolated community (village-scale)
- Simulates multiple generations of NPCs before the player enters
- No scripting or authored narrative
- Culture and meaning emerge purely from NPC behavior and memory

### Simulation Pillars
- **Speechless world**: All communication is behavioral and emotional
- **Emergent memory and culture**: No hardcoded lore or events
- **Player is one of many**: Not special, can act but not direct
- **History is lived**: Every trace is the result of real past behavior

---

## NPC Simulation

### Drives (Internal States)
Each NPC has fixed-intensity emotional drives:
- **Belonging**: Desire for closeness or shared behavior
- **Grief**: Response to loss or desecration
- **Curiosity**: Motivates exploration and novelty-seeking
- **Sustenance**: Hunger, rest, bodily needs
- **Shelter**: Comfort and safety
- **Pride**: Reinforcement of self-made or inherited behaviors

Drives rise over time or due to specific triggers, and actions can reduce them based on emotional or contextual relevance.

### Actions
All NPCs (and the player) can perform:
- Move
- Observe
- Give/Take
- Rest
- Build
- Plant
- Bury
- Gesture
- Follow

Actions are selected each tick based on the most urgent drive, filtered by available and contextually-relevant actions.

### Memory System
Each NPC has:
- **PerceptionBuffer**: Short-term list of recent actions and observations
- **MemoryEntry**: Stores individual witnessed or performed actions
- **ActionSequence**:
  - A list of timed `ActionStep`s with delays between them
  - Encodes meaningful sequences of behavior
- **MemoryEpisode**: A grouped reference to an ActionSequence, with:
  - `start_time`, `end_time`
  - `action_sequence_id`
  - `delta_drives`: emotional impact
  - `repetition_count`

### Drive Impact Model
- Each action has defined effects on one or more drives
- Effects are modified by:
  - Context (location, timing, object)
  - Relationship to the actor (affective trace)

---

## NPC Relationships

### Structure
Each NPC stores a `relationships` dictionary:
- `familiarity`: exposure level
- `affective_trace`: emotional impact history per drive
- `last_interaction`
- `episodic_count`

### Emergent Meaning Only
- No roles or labels (e.g., "friend", "parent")
- Relationships form purely from interaction outcomes and emotional memory

---

## Behavior Propagation

### Observation & Imitation
- NPCs observe others performing sequences
- Store `WitnessedSequence` if:
  - Seen multiple times
  - Actor is emotionally significant
  - Sequence appears emotionally effective

### Reproduction
- If an NPC imitates a witnessed sequence and it reduces their active drive:
  - Becomes a new MemoryEpisode
  - Can be repeated, refined, and passed on

### Convergence Detection
- Multiple NPCs performing similar actions:
  - In same space
  - In same time window
- Observers treat this as significant
- Increases likelihood of imitation and memory retention

---

## Cultural Divergence

### Mutation & Drift
- Imitation is imperfect
- Drive differences cause divergent emotional impact
- Isolation causes local convergence, global divergence

### Outcome
- No fixed rituals
- Traditions evolve through:
  - Repeated behavior
  - Emotional relevance
  - Multi-generational imitation

---

## Player Role
- The player enters the world after generations of simulation
- Treated as any other NPC
- Can perform the same actions
- May influence or adopt cultural behavior
- All discovery is environmental and observational

---

## Primitive Action Selection (Bootstrapping Behavior)

At simulation start:
- NPCs use primitive heuristics tied to drive intensity and environmental context
- No pre-existing memory or culture needed

Example behaviors:
- Belonging: move toward others, follow
- Curiosity: wander, observe unknowns
- Sustenance: rest in safe places
- Shelter: seek covered areas
- Pride: perform unique gestures, build

Each behavior creates memory, which later becomes repeatable and imitable.

---

## Objects & World Interaction

### Object Categories
| Category     | Description | Interactions |
|--------------|-------------|--------------|
| Food         | Consumables for sustenance | `take`, `give`, `observe` |
| Structures   | Built shelters or restful places | `build`, `rest`, `observe` |
| Tools/Artifacts | Created or found symbolic objects | `give`, `gesture with`, `observe`, `place` |
| Burials      | Grave sites, memory traces | `bury`, `observe`, `gesture`, `rest nearby` |
| Plants       | Growable, living interaction | `plant`, `observe` |
| Markers      | Symbolic land features (e.g., stones, shrines) | `observe`, `gesture`, `place`, `copy` |

### Object Properties
Each object includes:
- `category`
- `position`
- `created_by`
- `tags` (e.g., "restful", "sacred")
- `history` (interactions over time)

### Drive Relevance
Objects reduce drives based on category and tags:
- `Food` → sustenance
- `Structure` → shelter
- `Burials` → grief
- `Shared objects` → belonging
- `Unique creations` → pride
- `Unknown or rare` → curiosity

Objects become culturally meaningful through repeated, emotional interaction.

---

This spec defines the foundational mechanics for a fully emergent, history-driven simulation. All story and meaning arise from systems, memory, and interpretation.

