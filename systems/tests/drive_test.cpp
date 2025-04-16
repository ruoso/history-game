#include <gtest/gtest.h>
#include <history_game/datamodel/npc/drive.h>
#include <history_game/datamodel/npc/npc.h>
#include <history_game/systems/drives/drive_dynamics.h>
#include <history_game/systems/drives/drive_impact.h>
#include <history_game/datamodel/memory/memory_entry.h>
#include <history_game/datamodel/action/action_type.h>

// Use namespaces to avoid repetition, but only up to two levels
using namespace history_game::datamodel;
// Don't use systems namespace due to name conflicts

// Test drive dynamics
TEST(DriveDynamicsTest, UpdateDrive) {
    // Create a drive
    npc::Drive hunger(npc::drive::Sustenance{}, 50.0f);
    
    // Create parameters
    history_game::systems::drives::DriveParameters params(
        0.2f,  // Base growth rate
        0.5f,  // Intensity factor
        {
            { npc::DriveType{npc::drive::Sustenance{}}, 1.5f }  // Sustenance grows 50% faster
        }
    );
    
    // Update the drive over 10 ticks
    npc::Drive updated = history_game::systems::drives::drive_dynamics_system::updateDrive(hunger, params, 10);
    
    // Check that intensity increased
    EXPECT_GT(updated.intensity, hunger.intensity);
    
    // Check that it's still the same type
    EXPECT_TRUE(std::holds_alternative<npc::drive::Sustenance>(updated.type));
}

// Test drive impact context creation
TEST(DriveImpactTest, ActionContext) {
    // Create entity
    entity::Entity entity("test_entity", world::Position(10.0f, 20.0f));
    auto entity_ref = entity::Entity::storage::make_entity(std::move(entity));
    
    // Create NPCIdentity
    npc::NPCIdentity identity(entity_ref);
    auto identity_ref = npc::NPCIdentity::storage::make_entity(std::move(identity));
    
    // Create NPC
    std::vector<npc::Drive> drives = {
        npc::Drive(npc::drive::Sustenance{}, 50.0f),
        npc::Drive(npc::drive::Curiosity{}, 60.0f)
    };
    
    memory::PerceptionBuffer buffer({});
    auto perception = memory::PerceptionBuffer::storage::make_entity(std::move(buffer));
    
    npc::NPC npc(identity_ref, drives, perception, {}, {}, {});
    auto npc_ref = npc::NPC::storage::make_entity(std::move(npc));
    
    // Create a memory entry
    memory::MemoryEntry entry(
        100,  // timestamp
        identity_ref,
        action::action_type::Observe{},
        entity_ref  // observing the original entity
    );
    auto memory_ref = memory::MemoryEntry::storage::make_entity(std::move(entry));
    
    // Create action context
    history_game::datamodel::drives::ActionContext context(npc_ref, memory_ref, 100);
    
    // Check context
    EXPECT_EQ(context.observer, npc_ref);
    EXPECT_EQ(context.memory, memory_ref);
    EXPECT_EQ(context.current_time, 100);
}

// Test drive impact evaluation for Observe action
TEST(DriveImpactTest, ObserveImpact) {
    // Set up an observation context
    entity::Entity entity("test_entity", world::Position(10.0f, 20.0f));
    auto entity_ref = entity::Entity::storage::make_entity(std::move(entity));
    
    npc::NPCIdentity identity(entity_ref);
    auto identity_ref = npc::NPCIdentity::storage::make_entity(std::move(identity));
    
    std::vector<npc::Drive> drives = {
        npc::Drive(npc::drive::Curiosity{}, 60.0f)  // High curiosity
    };
    
    memory::PerceptionBuffer buffer({});
    auto perception = memory::PerceptionBuffer::storage::make_entity(std::move(buffer));
    
    npc::NPC npc(identity_ref, drives, perception, {}, {}, {});
    auto npc_ref = npc::NPC::storage::make_entity(std::move(npc));
    
    // Create a memory entry for observation
    memory::MemoryEntry entry(
        100,
        identity_ref,
        action::action_type::Observe{},
        entity_ref
    );
    auto memory_ref = memory::MemoryEntry::storage::make_entity(std::move(entry));
    
    // Create action context
    drives::ActionContext context(npc_ref, memory_ref, 100);
    
    // Get impacts
    std::vector<npc::Drive> impacts = drives::drive_impact_system::evaluateImpact(context);
    
    // Check impacts - Observe should reduce curiosity
    EXPECT_FALSE(impacts.empty());
    
    bool found_curiosity_impact = false;
    for (const auto& impact : impacts) {
        if (std::holds_alternative<npc::drive::Curiosity>(impact.type)) {
            found_curiosity_impact = true;
            // Curiosity impact should be negative (reducing curiosity)
            EXPECT_LT(impact.intensity, 0);
        }
    }
    
    EXPECT_TRUE(found_curiosity_impact);
}