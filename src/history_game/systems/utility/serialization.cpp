#include <filesystem>
#include "systems/utility/serialization.h"

namespace history_game {
namespace serialization {

/**
 * Get event type name using std::visit
 */
std::string getEventTypeName(const EventType& type) {
    return std::visit([](const auto& t) { return t.name; }, type);
}

// Event implementations
TickStartData::TickStartData(uint64_t time, uint64_t tick, uint32_t gen)
    : EventData(time), tick_number(tick), generation(gen) {}
    
json TickStartData::serialize() const {
    json j;
    j["timestamp"] = timestamp;
    j["type"] = event_type::TickStart{}.name;
    j["tick_number"] = tick_number;
    j["generation"] = generation;
    return j;
}

TickEndData::TickEndData(uint64_t time, uint64_t tick, uint32_t gen, 
                       uint32_t npcs, uint32_t objects)
    : EventData(time), tick_number(tick), generation(gen),
      npc_count(npcs), object_count(objects) {}
      
json TickEndData::serialize() const {
    json j;
    j["timestamp"] = timestamp;
    j["type"] = event_type::TickEnd{}.name;
    j["tick_number"] = tick_number;
    j["generation"] = generation;
    j["npc_count"] = npc_count;
    j["object_count"] = object_count;
    return j;
}

SimulationStartData::SimulationStartData(uint64_t time, uint32_t npcs, uint32_t objects,
                                float size, const std::vector<json>& entity_data)
    : EventData(time), npc_count(npcs), object_count(objects), world_size(size), entities(entity_data) {}
      
json SimulationStartData::serialize() const {
    json j;
    j["timestamp"] = timestamp;
    j["type"] = event_type::SimulationStart{}.name;
    j["npc_count"] = npc_count;
    j["object_count"] = object_count;
    j["world_size"] = world_size;
    
    // Add entities array if provided
    if (!entities.empty()) {
        j["entities"] = entities;
    }
    
    return j;
}

SimulationEndData::SimulationEndData(uint64_t time, uint64_t ticks, uint32_t gen, 
                                 uint32_t npcs, uint32_t objects)
    : EventData(time), total_ticks(ticks), final_generation(gen),
      npc_count(npcs), object_count(objects) {}
      
json SimulationEndData::serialize() const {
    json j;
    j["timestamp"] = timestamp;
    j["type"] = event_type::SimulationEnd{}.name;
    j["total_ticks"] = total_ticks;
    j["final_generation"] = final_generation;
    j["npc_count"] = npc_count;
    j["object_count"] = object_count;
    return j;
}

// Entity Update Event implementations
EntityUpdateData::EntityUpdateData(uint64_t time, const std::string& id, 
                                 const std::string& type, const json& pos,
                                 const std::optional<json>& drives_data,
                                 const std::optional<std::string>& action)
    : EventData(time), entity_id(id), entity_type(type), position(pos),
      drives(drives_data), current_action(action) {}
      
json EntityUpdateData::serialize() const {
    json j;
    j["timestamp"] = timestamp;
    j["type"] = "ENTITY_UPDATE";
    j["entity_id"] = entity_id;
    j["entity_type"] = entity_type;
    j["position"] = position;
    
    if (drives) {
        j["drives"] = drives.value();
    }
    
    if (current_action) {
        j["current_action"] = current_action.value();
    }
    
    return j;
}

// Action Execution Event implementations
ActionExecutionData::ActionExecutionData(uint64_t time, const std::string& id,
                                         const std::string& action,
                                         const std::optional<std::string>& target)
    : EventData(time), entity_id(id), action_type(action), target_id(target) {}
      
json ActionExecutionData::serialize() const {
    json j;
    j["timestamp"] = timestamp;
    j["type"] = "ACTION_EXECUTION";
    j["entity_id"] = entity_id;
    j["action_type"] = action_type;
    
    if (target_id) {
        j["target_id"] = target_id.value();
    }
    
    return j;
}

/**
 * Serialize any event using std::visit
 */
json serializeEvent(const SimulationEvent& event) {
    return std::visit([](const auto& e) -> json { return e.serialize(); }, event);
}

/**
 * Factory functions to create events
 */
SimulationEvent createTickStartEvent(uint64_t time, uint64_t tick, uint32_t gen) {
    return TickStartData(time, tick, gen);
}

SimulationEvent createTickEndEvent(uint64_t time, uint64_t tick, uint32_t gen, 
                                 uint32_t npcs, uint32_t objects) {
    return TickEndData(time, tick, gen, npcs, objects);
}

SimulationEvent createSimulationStartEvent(uint64_t time, uint32_t npcs, uint32_t objects,
                                float world_size, const std::vector<json>& entities) {
    return SimulationStartData(time, npcs, objects, world_size, entities);
}

SimulationEvent createSimulationEndEvent(uint64_t time, uint64_t ticks, uint32_t gen, 
                                       uint32_t npcs, uint32_t objects) {
    return SimulationEndData(time, ticks, gen, npcs, objects);
}

SimulationEvent createEntityUpdateEvent(uint64_t time, const std::string& id, 
                                     const std::string& type, const json& position,
                                     const std::optional<json>& drives,
                                     const std::optional<std::string>& action) {
    return EntityUpdateData(time, id, type, position, drives, action);
}

SimulationEvent createActionExecutionEvent(uint64_t time, const std::string& id,
                                         const std::string& action,
                                         const std::optional<std::string>& target) {
    return ActionExecutionData(time, id, action, target);
}

/**
 * SimulationLogger implementation
 */

void SimulationLogger::write(const json& data) {
    if (!is_initialized) return;
    
    // If we've already written events, prepend a comma
    if (has_events) {
        file_stream << "," << std::endl;
    }
    
    file_stream << data.dump(2); // 2-space indentation
    file_stream.flush();
    has_events = true;
}

bool SimulationLogger::initialize(const std::string& file_path) {
    // Create directory if it doesn't exist
    std::filesystem::path path(file_path);
    std::filesystem::create_directories(path.parent_path());
    
    file_stream.open(file_path);
    is_initialized = file_stream.is_open();
    output_path = file_path;
    has_events = false;
    
    if (is_initialized) {
        // Write a JSON array start
        file_stream << "[" << std::endl;
        file_stream.flush();
    }
    
    return is_initialized;
}

void SimulationLogger::logEvent(const SimulationEvent& event) {
    if (!is_initialized) return;
    
    // Serialize and write the event
    write(serializeEvent(event));
}

void SimulationLogger::shutdown() {
    if (!is_initialized) return;
    
    // Write a JSON array end
    file_stream << std::endl << "]" << std::endl;
    file_stream.flush();
    file_stream.close();
    is_initialized = false;
}

bool SimulationLogger::isInitialized() const {
    return is_initialized;
}

std::string SimulationLogger::getOutputPath() const {
    return output_path;
}

} // namespace serialization
} // namespace history_game