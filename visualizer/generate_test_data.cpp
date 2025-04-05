#include <iostream>
#include <fstream>
#include <random>
#include <nlohmann/json.hpp>
#include <chrono>

using json = nlohmann::json;

// Constants for simulation
const int WORLD_SIZE = 1000;
const int NUM_NPCS = 20;
const int NUM_OBJECTS = 30;
const int NUM_TICKS = 100;

// Random generators
std::random_device rd;
std::mt19937 gen(rd());
std::uniform_real_distribution<> pos_dist(0, WORLD_SIZE);
std::uniform_real_distribution<> drive_dist(0, 1);
std::uniform_int_distribution<> npc_dist(0, NUM_NPCS - 1);
std::uniform_int_distribution<> obj_dist(0, NUM_OBJECTS - 1);

// Generate a random position
json generatePosition() {
    json position;
    position["x"] = pos_dist(gen);
    position["y"] = pos_dist(gen);
    return position;
}

// Generate random drives
json generateDrives() {
    json drives = json::array();
    
    // Create some drive types
    const std::vector<std::string> drive_types = {
        "Belonging", "Curiosity", "Sustenance", "Shelter", "Pride"
    };
    
    for (const auto& type : drive_types) {
        json drive;
        drive["type"] = type;
        drive["value"] = drive_dist(gen);
        drives.push_back(drive);
    }
    
    return drives;
}

// Generate a random action
std::string generateAction() {
    const std::vector<std::string> actions = {
        "Move", "Observe", "Give", "Take", "Rest", "Build", "Plant", "Bury", "Gesture", "Follow"
    };
    
    std::uniform_int_distribution<> action_dist(0, actions.size() - 1);
    return actions[action_dist(gen)];
}

int main() {
    // Current timestamp
    uint64_t current_time = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    
    // Create JSON array for events
    json events = json::array();
    
    // Add simulation start event
    json start_event;
    start_event["timestamp"] = current_time;
    start_event["type"] = "SIMULATION_START";
    start_event["npc_count"] = NUM_NPCS;
    start_event["object_count"] = NUM_OBJECTS;
    events.push_back(start_event);
    
    // Create NPCs and objects with initial positions
    std::vector<json> npcs;
    std::vector<json> objects;
    
    for (int i = 0; i < NUM_NPCS; i++) {
        json npc;
        npc["id"] = "npc_" + std::to_string(i);
        npc["position"] = generatePosition();
        npc["drives"] = generateDrives();
        npcs.push_back(npc);
    }
    
    for (int i = 0; i < NUM_OBJECTS; i++) {
        json object;
        object["id"] = (i % 2 == 0) ? "food_" + std::to_string(i) : "shelter_" + std::to_string(i);
        object["position"] = generatePosition();
        objects.push_back(object);
    }
    
    // Generate events for each tick
    for (int tick = 0; tick < NUM_TICKS; tick++) {
        current_time += 100; // 100ms per tick
        
        // Add tick start event
        json tick_start;
        tick_start["timestamp"] = current_time;
        tick_start["type"] = "TICK_START";
        tick_start["tick_number"] = tick;
        tick_start["generation"] = tick / 10; // 10 ticks per generation
        events.push_back(tick_start);
        
        // Update and log entity states (every 5 ticks for efficiency)
        if (tick % 5 == 0) {
            // Update NPCs
            for (auto& npc : npcs) {
                // Update position slightly
                json& pos = npc["position"];
                std::normal_distribution<> move_dist(0, 10.0);
                
                double new_x = std::max(0.0, std::min(WORLD_SIZE * 1.0, 
                    static_cast<double>(pos["x"]) + move_dist(gen)));
                double new_y = std::max(0.0, std::min(WORLD_SIZE * 1.0, 
                    static_cast<double>(pos["y"]) + move_dist(gen)));
                
                pos["x"] = new_x;
                pos["y"] = new_y;
                
                // Update drives slightly
                for (auto& drive : npc["drives"]) {
                    double current = drive["value"];
                    std::normal_distribution<> drive_change(-0.05, 0.1);
                    double new_value = std::max(0.0, std::min(1.0, current + drive_change(gen)));
                    drive["value"] = new_value;
                }
                
                // Assign a random action
                npc["current_action"] = generateAction();
                
                // Log the entity update
                json entity_update;
                entity_update["timestamp"] = current_time;
                entity_update["type"] = "ENTITY_UPDATE";
                entity_update["entity_id"] = npc["id"];
                entity_update["entity_type"] = "NPC";
                entity_update["position"] = npc["position"];
                entity_update["drives"] = npc["drives"];
                entity_update["current_action"] = npc["current_action"];
                events.push_back(entity_update);
            }
            
            // Update objects (less frequently)
            for (auto& object : objects) {
                // Log the entity update
                json entity_update;
                entity_update["timestamp"] = current_time;
                entity_update["type"] = "ENTITY_UPDATE";
                entity_update["entity_id"] = object["id"];
                entity_update["entity_type"] = "Object";
                entity_update["position"] = object["position"];
                events.push_back(entity_update);
            }
        }
        
        // Generate some random actions (about 1/4 of NPCs per tick)
        std::uniform_int_distribution<> action_count_dist(NUM_NPCS / 6, NUM_NPCS / 3);
        int action_count = action_count_dist(gen);
        
        for (int i = 0; i < action_count; i++) {
            int npc_index = npc_dist(gen);
            json& actor = npcs[npc_index];
            
            // Create action execution event
            json action_event;
            action_event["timestamp"] = current_time + (i * 10); // Stagger the actions
            action_event["type"] = "ACTION_EXECUTION";
            action_event["entity_id"] = actor["id"];
            action_event["action_type"] = actor["current_action"];
            
            // Half of the actions have a target
            if (gen() % 2 == 0) {
                // Target can be another NPC or an object
                if (gen() % 2 == 0) {
                    // Target is an NPC
                    int target_index = npc_dist(gen);
                    while (target_index == npc_index) {
                        target_index = npc_dist(gen); // Avoid self-targeting
                    }
                    action_event["target_id"] = npcs[target_index]["id"];
                } else {
                    // Target is an object
                    action_event["target_id"] = objects[obj_dist(gen)]["id"];
                }
            }
            
            events.push_back(action_event);
        }
        
        // Add tick end event
        json tick_end;
        tick_end["timestamp"] = current_time + 99; // Just before the next tick
        tick_end["type"] = "TICK_END";
        tick_end["tick_number"] = tick;
        tick_end["generation"] = tick / 10;
        tick_end["npc_count"] = NUM_NPCS;
        tick_end["object_count"] = NUM_OBJECTS;
        events.push_back(tick_end);
    }
    
    // Add simulation end event
    current_time += 100;
    json end_event;
    end_event["timestamp"] = current_time;
    end_event["type"] = "SIMULATION_END";
    end_event["total_ticks"] = NUM_TICKS;
    end_event["final_generation"] = NUM_TICKS / 10;
    end_event["npc_count"] = NUM_NPCS;
    end_event["object_count"] = NUM_OBJECTS;
    events.push_back(end_event);
    
    // Write to file
    std::ofstream file("test_simulation_data.json");
    if (!file.is_open()) {
        std::cerr << "Failed to open output file" << std::endl;
        return 1;
    }
    
    file << events.dump(2);
    file.close();
    
    std::cout << "Generated test data with " << events.size() << " events" << std::endl;
    
    return 0;
}