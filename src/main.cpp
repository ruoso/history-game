#include <iostream>
#include <string>
#include <vector>
#include <random>
#include <map>
#include <set>
#include <filesystem>
#include <cpioo/managed_entity.hpp>
#include <spdlog/spdlog.h>
#include <chrono>

#include "entity/entity.h"
#include "world/world.h"
#include "world/simulation_clock.h"
#include "npc/npc.h"
#include "npc/drive.h"
#include "object/object.h"
#include "utility/log_init.h"
#include "utility/serialization.h"
#include "simulation/simulation_runner.h"
#include "memory/perception_buffer.h"

using namespace history_game;

// Helper to create a unique ID with a prefix
std::string createId(const std::string& prefix) {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<> dis(1000, 9999);
    
    return prefix + "_" + std::to_string(dis(gen));
}

// Create a position with random coordinates
Position createRandomPosition(float x_min, float x_max, float y_min, float y_max) {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    
    std::uniform_real_distribution<> x_dis(x_min, x_max);
    std::uniform_real_distribution<> y_dis(y_min, y_max);
    
    return Position(x_dis(gen), y_dis(gen));
}

// Create an entity with a random position
Entity::ref_type createEntity(const std::string& id_prefix, float x_min, float x_max, float y_min, float y_max) {
    Entity entity(createId(id_prefix), createRandomPosition(x_min, x_max, y_min, y_max));
    return Entity::storage::make_entity(std::move(entity));
}

// Create an NPC with basic drives
NPC::ref_type createNPC(const std::string& id_prefix, float x_min, float x_max, float y_min, float y_max) {
    // Create the entity
    auto entity = createEntity(id_prefix, x_min, x_max, y_min, y_max);
    
    // Create NPCIdentity
    NPCIdentity identity(entity);
    auto identity_ref = NPCIdentity::storage::make_entity(std::move(identity));
    
    // Create initial drives with random intensities
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_real_distribution<> intensity_dis(10.0f, 40.0f);
    
    std::vector<Drive> drives = {
        Drive(drive::Sustenance{}, intensity_dis(gen)),
        Drive(drive::Shelter{}, intensity_dis(gen)),
        Drive(drive::Belonging{}, intensity_dis(gen)),
        Drive(drive::Curiosity{}, intensity_dis(gen)),
        Drive(drive::Pride{}, intensity_dis(gen))
    };
    
    // Create empty perception buffer
    PerceptionBuffer buffer({});
    auto perception = PerceptionBuffer::storage::make_entity(std::move(buffer));
    
    // Create the NPC
    NPC npc(identity_ref, drives, perception, {}, {}, {});
    return NPC::storage::make_entity(std::move(npc));
}

// Create a food object
WorldObject::ref_type createFoodObject(const std::string& id_prefix, 
                                      float x_min, float x_max, 
                                      float y_min, float y_max,
                                      const NPCIdentity::ref_type& creator) {
    // Create the entity
    auto entity = createEntity(id_prefix, x_min, x_max, y_min, y_max);
    
    // Create the object
    WorldObject obj(entity, object_category::Food{}, creator);
    return WorldObject::storage::make_entity(std::move(obj));
}

// Create a structure object
WorldObject::ref_type createStructureObject(const std::string& id_prefix, 
                                           float x_min, float x_max, 
                                           float y_min, float y_max,
                                           const NPCIdentity::ref_type& creator) {
    // Create the entity
    auto entity = createEntity(id_prefix, x_min, x_max, y_min, y_max);
    
    // Create the object
    WorldObject obj(entity, object_category::Structure{}, creator);
    return WorldObject::storage::make_entity(std::move(obj));
}

int main() {
    // Initialize logging
    log_init::initialize("debug", "simulation.log");
    
    // Create an output directory for logs and serialization if it doesn't exist
    std::filesystem::create_directories("output");
    
    // Initialize serialization logger
    serialization::SimulationLogger sim_logger;
    bool logger_initialized = sim_logger.initialize("output/simulation_events.json");
    if (!logger_initialized) {
        spdlog::error("Failed to initialize simulation event logger");
        return 1;
    }
    
    // Create a simulation clock
    SimulationClock clock(0, 1, 100);  // Start at tick 0, generation 1, 100 ticks per generation
    auto clock_ref = SimulationClock::storage::make_entity(std::move(clock));
    
    // Create a larger world space
    const float WORLD_SIZE = 1000.0f;
    
    // Create 100 NPCs distributed across the world space
    std::vector<NPC::ref_type> npcs;
    for (int i = 0; i < 100; ++i) {
        npcs.push_back(createNPC("npc", 0.0f, WORLD_SIZE, 0.0f, WORLD_SIZE));
    }
    
    // Create some initial objects
    std::vector<WorldObject::ref_type> objects;
    
    // Random device for object creation
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> npc_dis(0, npcs.size() - 1);
    
    // Add food objects (50) spread across the world space
    for (int i = 0; i < 50; ++i) {
        // Randomly select an NPC to be the creator
        int npc_idx = npc_dis(gen);
        objects.push_back(createFoodObject("food", 0.0f, WORLD_SIZE, 0.0f, WORLD_SIZE, npcs[npc_idx]->identity));
    }
    
    // Add structure objects (50) spread across the world space
    for (int i = 0; i < 50; ++i) {
        // Randomly select an NPC to be the creator
        int npc_idx = npc_dis(gen);
        objects.push_back(createStructureObject("shelter", 0.0f, WORLD_SIZE, 0.0f, WORLD_SIZE, npcs[npc_idx]->identity));
    }
    
    // Create the initial world state
    World world(clock_ref, npcs, objects);
    auto world_ref = World::storage::make_entity(std::move(world));
    
    // Ready to start simulation
    spdlog::info("Starting simulation with {} NPCs and {} objects", 
                npcs.size(), objects.size());
    
    // Prepare entity data for simulation start event
    std::vector<serialization::json> entity_data;
    
    // Add all NPCs to the entity list
    for (const auto& npc : npcs) {
        serialization::json npc_json;
        npc_json["id"] = npc->identity->entity->id;
        npc_json["type"] = "NPC";
        
        // Add position data
        serialization::json position;
        position["x"] = npc->identity->entity->position.x;
        position["y"] = npc->identity->entity->position.y;
        npc_json["position"] = position;
        
        // Add drives data
        serialization::json drives_json = serialization::json::array();
        for (const auto& drive : npc->drives) {
            serialization::json drive_json;
            drive_json["type"] = drive_dynamics_system::get_drive_name(drive.type);
            drive_json["value"] = drive.intensity;
            drives_json.push_back(drive_json);
        }
        npc_json["drives"] = drives_json;
        
        // Add to entity list
        entity_data.push_back(npc_json);
    }
    
    // Add all objects to the entity list
    for (const auto& obj : objects) {
        serialization::json obj_json;
        obj_json["id"] = obj->entity->id;
        
        // Determine type based on category
        std::string type = "Object";
        std::visit([&](const auto& category) {
            using CategoryType = std::decay_t<decltype(category)>;
            if constexpr (std::is_same_v<CategoryType, object_category::Food>) {
                type = "Food";
            } else if constexpr (std::is_same_v<CategoryType, object_category::Structure>) {
                type = "Structure";
            }
        }, obj->category);
        
        obj_json["type"] = type;
        
        // Add position data
        serialization::json position;
        position["x"] = obj->entity->position.x;
        position["y"] = obj->entity->position.y;
        obj_json["position"] = position;
        
        // Add to entity list
        entity_data.push_back(obj_json);
    }
    
    // Log simulation start event with world size and entities
    uint64_t current_time = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    sim_logger.logEvent(serialization::createSimulationStartEvent(
        current_time, npcs.size(), objects.size(), WORLD_SIZE, entity_data));
    
    // Create simulation parameters
    NPCUpdateParams params(
        DriveParameters(0.2f, 0.5f),  // Drive dynamics
        0.6f,  // Familiarity preference
        0.7f,  // Social preference 
        0.3f,  // Randomness
        0.3f,  // Significance threshold
        3,     // Max sequence gap
        2      // Min sequence length
    );
    
    // Run the simulation for 200 ticks (2 generations)
    // Shorter run for development to avoid long build times
    World::ref_type final_world = simulation_runner_system::runSimulation(
        world_ref, 
        200,  // 200 ticks for development
        params, 
        100.0f, // Increased perception range for larger world
        &sim_logger // Pass the serialization logger
    );
    
    // Log simulation end event
    current_time = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    sim_logger.logEvent(serialization::createSimulationEndEvent(
        current_time, 
        final_world->clock->current_tick,
        final_world->clock->current_generation,
        final_world->npcs.size(), 
        final_world->objects.size()));
    
    // Close the serialization logger
    sim_logger.shutdown();
    
    // Update our output reference without assignment
    // Use final_world for outputs below
    
    // Print final state summary
    spdlog::info("Simulation completed");
    spdlog::info("Final tick: {}", final_world->clock->current_tick);
    spdlog::info("Final generation: {}", final_world->clock->current_generation);
    spdlog::info("NPCs: {}", final_world->npcs.size());
    spdlog::info("Objects: {}", final_world->objects.size());
    
    // Print summary statistics instead of individual NPCs
    spdlog::info("NPC Population Summary:");
    
    // Count NPCs by action type
    std::map<std::string, int> action_counts;
    int no_action_count = 0;
    
    // Count perception and memory statistics
    int total_perceptions = 0;
    int total_episodes = 0;
    
    // Count average drive levels
    std::map<std::string, float> total_drive_values;
    std::map<std::string, int> drive_counts;
    
    for (const auto& npc : final_world->npcs) {
        // Count actions
        if (npc->identity->current_action) {
            std::string action_name = action_selection_system::get_action_name(npc->identity->current_action.value());
            action_counts[action_name]++;
        } else {
            no_action_count++;
        }
        
        // Count perceptions and memories
        total_perceptions += npc->perception->recent_perceptions.size();
        total_episodes += npc->episodic_memory.size();
        
        // Sum drive values
        for (const auto& drive : npc->drives) {
            std::string drive_name = drive_dynamics_system::get_drive_name(drive.type);
            total_drive_values[drive_name] += drive.intensity;
            drive_counts[drive_name]++;
        }
    }
    
    // Print action statistics
    spdlog::info("Action Distribution:");
    for (const auto& [action, count] : action_counts) {
        spdlog::info("  {}: {} NPCs ({:.1f}%)", 
                    action, count, (count * 100.0f) / final_world->npcs.size());
    }
    if (no_action_count > 0) {
        spdlog::info("  No Action: {} NPCs ({:.1f}%)", 
                    no_action_count, (no_action_count * 100.0f) / final_world->npcs.size());
    }
    
    // Print memory statistics
    float avg_perceptions = total_perceptions / static_cast<float>(final_world->npcs.size());
    float avg_episodes = total_episodes / static_cast<float>(final_world->npcs.size());
    spdlog::info("Memory Statistics:");
    spdlog::info("  Average perception buffer size: {:.2f}", avg_perceptions);
    spdlog::info("  Average episodic memories: {:.2f}", avg_episodes);
    spdlog::info("  Total episodic memories: {}", total_episodes);
    
    // Print drive statistics
    spdlog::info("Average Drive Levels:");
    for (const auto& [drive_name, total] : total_drive_values) {
        float avg = total / drive_counts[drive_name];
        spdlog::info("  {}: {:.2f}", drive_name, avg);
    }
    
    // Print 5 random NPCs for a more detailed view
    spdlog::info("\nDetailed view of 5 random NPCs:");
    std::uniform_int_distribution<> sample_dis(0, final_world->npcs.size() - 1);
    
    std::set<int> sampled_indices;
    while (sampled_indices.size() < 5 && sampled_indices.size() < final_world->npcs.size()) {
        sampled_indices.insert(sample_dis(gen));
    }
    
    for (int idx : sampled_indices) {
        const auto& npc = final_world->npcs[idx];
        spdlog::info("NPC {}: Position ({:.2f}, {:.2f})", 
                    npc->identity->entity->id,
                    npc->identity->entity->position.x,
                    npc->identity->entity->position.y);
        
        // Print drive levels
        for (const auto& drive : npc->drives) {
            std::string drive_name = drive_dynamics_system::get_drive_name(drive.type);
            spdlog::info("  Drive {}: {:.2f}", drive_name, drive.intensity);
        }
        
        // Print memory stats
        spdlog::info("  Perception buffer: {} entries", npc->perception->recent_perceptions.size());
        spdlog::info("  Episodic memories: {} episodes", npc->episodic_memory.size());
        
        // Print current action if any
        if (npc->identity->current_action) {
            std::string action_name = action_selection_system::get_action_name(npc->identity->current_action.value());
            
            if (npc->identity->target_entity) {
                spdlog::info("  Current action: {} targeting entity {}", 
                            action_name, 
                            npc->identity->target_entity.value()->id);
            }
            else if (npc->identity->target_object) {
                spdlog::info("  Current action: {} targeting object {}", 
                            action_name, 
                            npc->identity->target_object.value()->entity->id);
            }
            else {
                spdlog::info("  Current action: {}", action_name);
            }
        }
        else {
            spdlog::info("  No current action");
        }
    }
    
    // Shutdown logging
    log_init::shutdown();
    
    return 0;
}