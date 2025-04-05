#include <iostream>
#include <string>
#include <vector>
#include <random>
#include <cpioo/managed_entity.hpp>
#include <spdlog/spdlog.h>

#include "entity/entity.h"
#include "world/world.h"
#include "world/simulation_clock.h"
#include "npc/npc.h"
#include "npc/drive.h"
#include "object/object.h"
#include "utility/log_init.h"
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
    
    // Create a simulation clock
    SimulationClock clock(0, 1, 100);  // Start at tick 0, generation 1, 100 ticks per generation
    auto clock_ref = SimulationClock::storage::make_entity(std::move(clock));
    
    // Create NPCs
    std::vector<NPC::ref_type> npcs;
    for (int i = 0; i < 5; ++i) {
        npcs.push_back(createNPC("npc", 0.0f, 100.0f, 0.0f, 100.0f));
    }
    
    // Create some initial objects
    std::vector<WorldObject::ref_type> objects;
    
    // Add some food objects
    for (int i = 0; i < 3; ++i) {
        objects.push_back(createFoodObject("food", 10.0f, 90.0f, 10.0f, 90.0f, npcs[0]->identity));
    }
    
    // Add some structure objects
    for (int i = 0; i < 2; ++i) {
        objects.push_back(createStructureObject("shelter", 20.0f, 80.0f, 20.0f, 80.0f, npcs[0]->identity));
    }
    
    // Create the initial world state
    World world(clock_ref, npcs, objects);
    auto world_ref = World::storage::make_entity(std::move(world));
    
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
    
    // Run the simulation for 20 ticks
    world_ref = simulation_runner_system::runSimulation(
        world_ref, 
        20,
        params, 
        15.0f  // Perception range
    );
    
    // Print final state summary
    spdlog::info("Simulation completed");
    spdlog::info("Final tick: {}", world_ref->clock->current_tick);
    spdlog::info("Final generation: {}", world_ref->clock->current_generation);
    spdlog::info("NPCs: {}", world_ref->npcs.size());
    spdlog::info("Objects: {}", world_ref->objects.size());
    
    // Print NPC state summary
    for (const auto& npc : world_ref->npcs) {
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