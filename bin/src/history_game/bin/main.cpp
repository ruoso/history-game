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

#include <history_game/datamodel/entity/entity.h>
#include <history_game/datamodel/world/world.h>
#include <history_game/datamodel/world/simulation_clock.h>
#include <history_game/datamodel/npc/npc.h>
#include <history_game/datamodel/npc/drive.h>
#include <history_game/datamodel/object/object.h>
#include <history_game/systems/utility/log_init.h>
#include <history_game/systems/utility/serialization.h>
#include <history_game/systems/simulation/simulation_runner.h>
#include <history_game/datamodel/memory/perception_buffer.h>

namespace history_game::bin {

// Helper to create a unique ID with a prefix
std::string createId(const std::string& prefix) {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<> dis(1000, 9999);
    
    return prefix + "_" + std::to_string(dis(gen));
}

// Create a position with random coordinates
datamodel::world::Position createRandomPosition(float x_min, float x_max, float y_min, float y_max) {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    
    std::uniform_real_distribution<> x_dis(x_min, x_max);
    std::uniform_real_distribution<> y_dis(y_min, y_max);
    
    return datamodel::world::Position(x_dis(gen), y_dis(gen));
}

// Create an entity with a random position
datamodel::entity::Entity::ref_type createEntity(const std::string& id_prefix, float x_min, float x_max, float y_min, float y_max) {
    datamodel::entity::Entity entity(createId(id_prefix), createRandomPosition(x_min, x_max, y_min, y_max));
    return datamodel::entity::Entity::storage::make_entity(std::move(entity));
}

// Create an NPC with basic drives
datamodel::npc::NPC::ref_type createNPC(const std::string& id_prefix, float x_min, float x_max, float y_min, float y_max) {
    // Create the entity
    auto entity = createEntity(id_prefix, x_min, x_max, y_min, y_max);
    
    // Create NPCIdentity
    datamodel::npc::NPCIdentity identity(entity);
    auto identity_ref = datamodel::npc::NPCIdentity::storage::make_entity(std::move(identity));
    
    // Create initial drives with random intensities
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_real_distribution<> intensity_dis(10.0f, 40.0f);
    
    std::vector<datamodel::npc::Drive> drives = {
        datamodel::npc::Drive(datamodel::npc::drive::Sustenance{}, intensity_dis(gen)),
        datamodel::npc::Drive(datamodel::npc::drive::Shelter{}, intensity_dis(gen)),
        datamodel::npc::Drive(datamodel::npc::drive::Belonging{}, intensity_dis(gen)),
        datamodel::npc::Drive(datamodel::npc::drive::Curiosity{}, intensity_dis(gen)),
        datamodel::npc::Drive(datamodel::npc::drive::Pride{}, intensity_dis(gen))
    };
    
    // Create empty perception buffer
    datamodel::memory::PerceptionBuffer buffer({});
    auto perception = datamodel::memory::PerceptionBuffer::storage::make_entity(std::move(buffer));
    
    // Create the NPC
    datamodel::npc::NPC npc(identity_ref, drives, perception, {}, {}, {});
    return datamodel::npc::NPC::storage::make_entity(std::move(npc));
}

// Create a food object
datamodel::object::WorldObject::ref_type createFoodObject(const std::string& id_prefix, 
                                      float x_min, float x_max, 
                                      float y_min, float y_max,
                                      const datamodel::npc::NPCIdentity::ref_type& creator) {
    // Create the entity
    auto entity = createEntity(id_prefix, x_min, x_max, y_min, y_max);
    
    // Create the object
    datamodel::object::WorldObject obj(entity, datamodel::object::object_category::Food{}, creator);
    return datamodel::object::WorldObject::storage::make_entity(std::move(obj));
}

// Create a structure object
datamodel::object::WorldObject::ref_type createStructureObject(const std::string& id_prefix, 
                                           float x_min, float x_max, 
                                           float y_min, float y_max,
                                           const datamodel::npc::NPCIdentity::ref_type& creator) {
    // Create the entity
    auto entity = createEntity(id_prefix, x_min, x_max, y_min, y_max);
    
    // Create the object
    datamodel::object::WorldObject obj(entity, datamodel::object::object_category::Structure{}, creator);
    return datamodel::object::WorldObject::storage::make_entity(std::move(obj));
}

int main() {
    // Initialize logging
    systems::utility::log_initialize("debug", "simulation.log");
    
    // Create an output directory for logs and serialization if it doesn't exist
    std::filesystem::create_directories("output");
    
    // Initialize serialization logger
    systems::utility::SimulationLogger sim_logger;
    bool logger_initialized = sim_logger.initialize("output/simulation_events.json");
    if (!logger_initialized) {
        spdlog::error("Failed to initialize simulation event logger");
        return 1;
    }
    
    // Create a simulation clock
    datamodel::world::SimulationClock clock(0, 1, 100);  // Start at tick 0, generation 1, 100 ticks per generation
    auto clock_ref = datamodel::world::SimulationClock::storage::make_entity(std::move(clock));
    
    // Create a larger world space
    const float WORLD_SIZE = 1000.0f;
    
    // Create 100 NPCs distributed across the world space
    std::vector<datamodel::npc::NPC::ref_type> npcs;
    for (int i = 0; i < 100; ++i) {
        npcs.push_back(createNPC("npc", 0.0f, WORLD_SIZE, 0.0f, WORLD_SIZE));
    }
    
    // Create some initial objects
    std::vector<datamodel::object::WorldObject::ref_type> objects;
    
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
    datamodel::world::World world(clock_ref, npcs, objects);
    auto world_ref = datamodel::world::World::storage::make_entity(std::move(world));
    
    // Ready to start simulation
    spdlog::info("Starting simulation with {} NPCs and {} objects", 
                npcs.size(), objects.size());
    
    // Prepare entity data for simulation start event
    std::vector<systems::utility::json> entity_data;
    
    // Add all NPCs to the entity list
    for (const auto& npc : npcs) {
        systems::utility::json npc_json;
        npc_json["id"] = npc->identity->entity->id;
        npc_json["type"] = "NPC";
        
        // Add position data
        systems::utility::json position;
        position["x"] = npc->identity->entity->position.x;
        position["y"] = npc->identity->entity->position.y;
        npc_json["position"] = position;
        
        // Add drives data
        systems::utility::json drives_json = systems::utility::json::array();
        for (const auto& drive : npc->drives) {
            systems::utility::json drive_json;
            drive_json["type"] = systems::drives::drive_dynamics_system::get_drive_name(drive.type);
            drive_json["value"] = drive.intensity;
            drives_json.push_back(drive_json);
        }
        npc_json["drives"] = drives_json;
        
        // Add to entity list
        entity_data.push_back(npc_json);
    }
    
    // Add all objects to the entity list
    for (const auto& obj : objects) {
        systems::utility::json obj_json;
        obj_json["id"] = obj->entity->id;
        
        // Determine type based on category
        std::string type = "Object";
        std::visit([&](const auto& category) {
            using CategoryType = std::decay_t<decltype(category)>;
            if constexpr (std::is_same_v<CategoryType, datamodel::object::object_category::Food>) {
                type = "Food";
            } else if constexpr (std::is_same_v<CategoryType, datamodel::object::object_category::Structure>) {
                type = "Structure";
            }
        }, obj->category);
        
        obj_json["type"] = type;
        
        // Add position data
        systems::utility::json position;
        position["x"] = obj->entity->position.x;
        position["y"] = obj->entity->position.y;
        obj_json["position"] = position;
        
        // Add to entity list
        entity_data.push_back(obj_json);
    }
    
    // Log simulation start event with world size and entities
    uint64_t current_time = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    sim_logger.logEvent(systems::utility::createSimulationStartEvent(
        current_time, npcs.size(), objects.size(), WORLD_SIZE, entity_data));
    
    // Create simulation parameters
    systems::simulation::NPCUpdateParams params(
        systems::drives::DriveParameters(0.2f, 0.5f),  // Drive dynamics
        0.6f,  // Familiarity preference
        0.7f,  // Social preference 
        0.3f,  // Randomness
        0.3f,  // Significance threshold
        3,     // Max sequence gap
        2      // Min sequence length
    );
    
    // Run the simulation for 200 ticks (2 generations)
    // Shorter run for development to avoid long build times
    datamodel::world::World::ref_type final_world = systems::simulation::runSimulation(
        world_ref, 
        200,  // 200 ticks for development
        params, 
        100.0f, // Increased perception range for larger world
        &sim_logger // Pass the serialization logger
    );
    
    // Log simulation end event
    current_time = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    sim_logger.logEvent(systems::utility::createSimulationEndEvent(
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
            std::string action_name = systems::behavior::action_selection_system::get_action_name(npc->identity->current_action.value());
            action_counts[action_name]++;
        } else {
            no_action_count++;
        }
        
        // Count perceptions and memories
        total_perceptions += npc->perception->recent_perceptions.size();
        total_episodes += npc->episodic_memory.size();
        
        // Sum drive values
        for (const auto& drive : npc->drives) {
            std::string drive_name = systems::drives::drive_dynamics_system::get_drive_name(drive.type);
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
            std::string drive_name = systems::drives::drive_dynamics_system::get_drive_name(drive.type);
            spdlog::info("  Drive {}: {:.2f}", drive_name, drive.intensity);
        }
        
        // Print memory stats
        spdlog::info("  Perception buffer: {} entries", npc->perception->recent_perceptions.size());
        spdlog::info("  Episodic memories: {} episodes", npc->episodic_memory.size());
        
        // Print current action if any
        if (npc->identity->current_action) {
            std::string action_name = systems::behavior::action_selection_system::get_action_name(npc->identity->current_action.value());
            
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
    systems::utility::log_shutdown();
    
    return 0;
}

} // namespace history_game::bin

int main() {
    return history_game::bin::main();
}
