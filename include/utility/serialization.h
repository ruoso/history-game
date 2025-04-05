#ifndef HISTORY_GAME_SERIALIZATION_H
#define HISTORY_GAME_SERIALIZATION_H

#include <string>
#include <vector>
#include <map>
#include <variant>
#include <optional>
#include <concepts>
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>

namespace history_game {
namespace serialization {

using json = nlohmann::json;

/**
 * Event types using strong typing pattern
 */
namespace event_type {
    struct TickStart { static constexpr auto name = "TICK_START"; };
    struct TickEnd { static constexpr auto name = "TICK_END"; };
    struct Perception { static constexpr auto name = "PERCEPTION"; };
    struct ActionSelection { static constexpr auto name = "ACTION_SELECTION"; };
    struct ActionExecution { static constexpr auto name = "ACTION_EXECUTION"; };
    struct DriveUpdate { static constexpr auto name = "DRIVE_UPDATE"; };
    struct MemoryFormation { static constexpr auto name = "MEMORY_FORMATION"; };
    struct RelationshipUpdate { static constexpr auto name = "RELATIONSHIP_UPDATE"; };
    struct SimulationStart { static constexpr auto name = "SIMULATION_START"; };
    struct SimulationEnd { static constexpr auto name = "SIMULATION_END"; };
}

/**
 * Event type variant for all possible event types
 */
using EventType = std::variant<
    event_type::TickStart,
    event_type::TickEnd,
    event_type::Perception,
    event_type::ActionSelection,
    event_type::ActionExecution,
    event_type::DriveUpdate,
    event_type::MemoryFormation,
    event_type::RelationshipUpdate,
    event_type::SimulationStart,
    event_type::SimulationEnd
>;

/**
 * Get event type name using std::visit
 */
std::string getEventTypeName(const EventType& type);

/**
 * Base event data structure for all events
 */
struct EventData {
    uint64_t timestamp;
    
    EventData(uint64_t time) : timestamp(time) {}
    virtual ~EventData() = default;
    virtual json serialize() const = 0;
};

/**
 * Event for simulation tick start
 */
struct TickStartData : EventData {
    uint64_t tick_number;
    uint32_t generation;
    
    TickStartData(uint64_t time, uint64_t tick, uint32_t gen);
    json serialize() const override;
};

/**
 * Event for simulation tick end
 */
struct TickEndData : EventData {
    uint64_t tick_number;
    uint32_t generation;
    uint32_t npc_count;
    uint32_t object_count;
    
    TickEndData(uint64_t time, uint64_t tick, uint32_t gen, 
               uint32_t npcs, uint32_t objects);
    json serialize() const override;
};

/**
 * Event for simulation start
 */
struct SimulationStartData : EventData {
    uint32_t npc_count;
    uint32_t object_count;
    
    SimulationStartData(uint64_t time, uint32_t npcs, uint32_t objects);
    json serialize() const override;
};

/**
 * Event for simulation end
 */
struct SimulationEndData : EventData {
    uint64_t total_ticks;
    uint32_t final_generation;
    uint32_t npc_count;
    uint32_t object_count;
    
    SimulationEndData(uint64_t time, uint64_t ticks, uint32_t gen, 
                     uint32_t npcs, uint32_t objects);
    json serialize() const override;
};

/**
 * Event for entity state update (position, actions, etc.)
 */
struct EntityUpdateData : EventData {
    std::string entity_id;
    std::string entity_type; // "NPC" or "Object"
    json position;
    std::optional<json> drives;
    std::optional<std::string> current_action;
    
    EntityUpdateData(uint64_t time, const std::string& id, 
                    const std::string& type, const json& pos,
                    const std::optional<json>& drives = std::nullopt,
                    const std::optional<std::string>& action = std::nullopt);
    json serialize() const override;
};

/**
 * Event for action execution
 */
struct ActionExecutionData : EventData {
    std::string entity_id;
    std::string action_type;
    std::optional<std::string> target_id;
    
    ActionExecutionData(uint64_t time, const std::string& id,
                        const std::string& action,
                        const std::optional<std::string>& target = std::nullopt);
    json serialize() const override;
};

/**
 * Variant type for all event data types
 */
using SimulationEvent = std::variant<
    TickStartData,
    TickEndData,
    SimulationStartData,
    SimulationEndData,
    EntityUpdateData,
    ActionExecutionData
>;

/**
 * Serialize any event using std::visit
 */
json serializeEvent(const SimulationEvent& event);

/**
 * Factory functions to create events
 */
SimulationEvent createTickStartEvent(uint64_t time, uint64_t tick, uint32_t gen);
SimulationEvent createTickEndEvent(uint64_t time, uint64_t tick, uint32_t gen, 
                                 uint32_t npcs, uint32_t objects);
SimulationEvent createSimulationStartEvent(uint64_t time, uint32_t npcs, uint32_t objects);
SimulationEvent createSimulationEndEvent(uint64_t time, uint64_t ticks, uint32_t gen, 
                                       uint32_t npcs, uint32_t objects);
SimulationEvent createEntityUpdateEvent(uint64_t time, const std::string& id, 
                                     const std::string& type, const json& position,
                                     const std::optional<json>& drives = std::nullopt,
                                     const std::optional<std::string>& action = std::nullopt);
SimulationEvent createActionExecutionEvent(uint64_t time, const std::string& id,
                                         const std::string& action,
                                         const std::optional<std::string>& target = std::nullopt);

/**
 * Logger for simulation events
 */
class SimulationLogger {
private:
    std::ofstream file_stream;
    bool is_initialized = false;
    std::string output_path;
    bool has_events = false;
    
    // Write to the file and flush
    void write(const json& data);
    
public:
    SimulationLogger() = default;
    
    // Initialize the logger with a file path
    bool initialize(const std::string& file_path);
    
    // Log an event using the variant
    void logEvent(const SimulationEvent& event);
    
    // Shutdown the logger
    void shutdown();
    
    // Check if logger is initialized
    bool isInitialized() const;
    
    // Get the output path
    std::string getOutputPath() const;
};

} // namespace serialization
} // namespace history_game

#endif // HISTORY_GAME_SERIALIZATION_H