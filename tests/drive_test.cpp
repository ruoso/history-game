#include <gtest/gtest.h>
#include "datamodel/npc/drive.h"
#include "datamodel/npc/npc.h"
#include "systems/drives/drive_dynamics.h"
#include "systems/drives/drive_impact.h"
#include "datamodel/memory/memory_entry.h"
#include "datamodel/action/action_type.h"

using namespace history_game;

// Test drive dynamics
TEST(DriveDynamicsTest, UpdateDrive) {
    // Create a drive
    Drive hunger(drive::Sustenance{}, 50.0f);
    
    // Create parameters
    DriveParameters params(
        0.2f,  // Base growth rate
        0.5f,  // Intensity factor
        {
            { DriveType{drive::Sustenance{}}, 1.5f }  // Sustenance grows 50% faster
        }
    );
    
    // Update the drive over 10 ticks
    Drive updated = drive_dynamics_system::updateDrive(hunger, params, 10);
    
    // Check that intensity increased
    EXPECT_GT(updated.intensity, hunger.intensity);
    
    // Check that it's still the same type
    EXPECT_TRUE(std::holds_alternative<drive::Sustenance>(updated.type));
}

// Test drive impact context creation
TEST(DriveImpactTest, ActionContext) {
    // Create entity
    Entity entity("test_entity", Position(10.0f, 20.0f));
    auto entity_ref = Entity::storage::make_entity(std::move(entity));
    
    // Create NPCIdentity
    NPCIdentity identity(entity_ref);
    auto identity_ref = NPCIdentity::storage::make_entity(std::move(identity));
    
    // Create NPC
    std::vector<Drive> drives = {
        Drive(drive::Sustenance{}, 50.0f),
        Drive(drive::Curiosity{}, 60.0f)
    };
    
    PerceptionBuffer buffer({});
    auto perception = PerceptionBuffer::storage::make_entity(std::move(buffer));
    
    NPC npc(identity_ref, drives, perception, {}, {}, {});
    auto npc_ref = NPC::storage::make_entity(std::move(npc));
    
    // Create a memory entry
    MemoryEntry entry(
        100,  // timestamp
        identity_ref,
        action_type::Observe{},
        entity_ref  // observing the original entity
    );
    auto memory_ref = MemoryEntry::storage::make_entity(std::move(entry));
    
    // Create action context
    ActionContext context(npc_ref, memory_ref, 100);
    
    // Check context
    EXPECT_EQ(context.observer, npc_ref);
    EXPECT_EQ(context.memory, memory_ref);
    EXPECT_EQ(context.current_time, 100);
}

// Test drive impact evaluation for Observe action
TEST(DriveImpactTest, ObserveImpact) {
    // Set up an observation context
    Entity entity("test_entity", Position(10.0f, 20.0f));
    auto entity_ref = Entity::storage::make_entity(std::move(entity));
    
    NPCIdentity identity(entity_ref);
    auto identity_ref = NPCIdentity::storage::make_entity(std::move(identity));
    
    std::vector<Drive> drives = {
        Drive(drive::Curiosity{}, 60.0f)  // High curiosity
    };
    
    PerceptionBuffer buffer({});
    auto perception = PerceptionBuffer::storage::make_entity(std::move(buffer));
    
    NPC npc(identity_ref, drives, perception, {}, {}, {});
    auto npc_ref = NPC::storage::make_entity(std::move(npc));
    
    // Create a memory entry for observation
    MemoryEntry entry(
        100,
        identity_ref,
        action_type::Observe{},
        entity_ref
    );
    auto memory_ref = MemoryEntry::storage::make_entity(std::move(entry));
    
    // Create action context
    ActionContext context(npc_ref, memory_ref, 100);
    
    // Get impacts
    std::vector<Drive> impacts = drive_impact_system::evaluateImpact(context);
    
    // Check impacts - Observe should reduce curiosity
    EXPECT_FALSE(impacts.empty());
    
    bool found_curiosity_impact = false;
    for (const auto& impact : impacts) {
        if (std::holds_alternative<drive::Curiosity>(impact.type)) {
            found_curiosity_impact = true;
            // Curiosity impact should be negative (reducing curiosity)
            EXPECT_LT(impact.intensity, 0);
        }
    }
    
    EXPECT_TRUE(found_curiosity_impact);
}