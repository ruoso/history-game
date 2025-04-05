#include <gtest/gtest.h>
#include "entity/entity.h"
#include "world/position.h"
#include "npc/npc.h"
#include "npc/drive.h"
#include "memory/perception_buffer.h"

using namespace history_game;

// Test entity creation and immutability
TEST(EntityTest, CreateAndAccess) {
    // Create an entity
    Entity entity("test_entity", Position(10.0f, 20.0f));
    
    // Check fields
    EXPECT_EQ(entity.id, "test_entity");
    EXPECT_FLOAT_EQ(entity.position.x, 10.0f);
    EXPECT_FLOAT_EQ(entity.position.y, 20.0f);
}

// Test managed entity reference
TEST(EntityTest, ManagedReference) {
    // Create an entity through the storage system
    Entity entity("test_entity", Position(10.0f, 20.0f));
    auto ref = Entity::storage::make_entity(std::move(entity));
    
    // Access through reference
    EXPECT_EQ(ref->id, "test_entity");
    EXPECT_FLOAT_EQ(ref->position.x, 10.0f);
    EXPECT_FLOAT_EQ(ref->position.y, 20.0f);
}

// Test drive creation
TEST(DriveTest, CreateDrive) {
    // Create a drive with type and intensity
    Drive hunger(drive::Sustenance{}, 75.0f);
    
    // Check fields
    EXPECT_FLOAT_EQ(hunger.intensity, 75.0f);
    EXPECT_TRUE(std::holds_alternative<drive::Sustenance>(hunger.type));
}

// Test NPC creation
TEST(NPCTest, CreateNPC) {
    // Create components
    Entity entity("test_npc", Position(15.0f, 25.0f));
    auto entity_ref = Entity::storage::make_entity(std::move(entity));
    
    NPCIdentity identity(entity_ref);
    auto identity_ref = NPCIdentity::storage::make_entity(std::move(identity));
    
    std::vector<Drive> drives = {
        Drive(drive::Sustenance{}, 50.0f),
        Drive(drive::Curiosity{}, 60.0f)
    };
    
    PerceptionBuffer buffer({});
    auto perception = PerceptionBuffer::storage::make_entity(std::move(buffer));
    
    // Create NPC
    NPC npc(
        identity_ref,
        drives,
        perception,
        {},  // No episodic memories
        {},  // No observed behaviors
        {}   // No relationships
    );
    
    // Check fields
    EXPECT_EQ(npc.identity->entity->id, "test_npc");
    EXPECT_EQ(npc.drives.size(), 2);
    EXPECT_TRUE(std::holds_alternative<drive::Sustenance>(npc.drives[0].type));
    EXPECT_TRUE(std::holds_alternative<drive::Curiosity>(npc.drives[1].type));
    EXPECT_FLOAT_EQ(npc.drives[0].intensity, 50.0f);
    EXPECT_FLOAT_EQ(npc.drives[1].intensity, 60.0f);
    EXPECT_TRUE(npc.perception->recent_perceptions.empty());
    EXPECT_TRUE(npc.episodic_memory.empty());
}