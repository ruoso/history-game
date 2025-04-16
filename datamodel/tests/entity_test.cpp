#include <gtest/gtest.h>
#include <history_game/datamodel/entity/entity.h>
#include <history_game/datamodel/world/position.h>
#include <history_game/datamodel/npc/npc.h>
#include <history_game/datamodel/npc/drive.h>
#include <history_game/datamodel/memory/perception_buffer.h>

using namespace history_game::datamodel;

// Test entity::Entity creation and immutability
TEST(EntityTest, CreateAndAccess) {
    // Create an entity
    entity::Entity entity("test_entity",  world::Position(10.0f, 20.0f));
    
    // Check fields
    EXPECT_EQ(entity.id, "test_entity");
    EXPECT_FLOAT_EQ(entity.position.x, 10.0f);
    EXPECT_FLOAT_EQ(entity.position.y, 20.0f);
}

// Test managed entity::Entity reference
TEST(EntityTest, ManagedReference) {
    // Create an entity::Entity through the storage system
    entity::Entity entity("test_entity",  world::Position(10.0f, 20.0f));
    auto ref = entity::Entity::storage::make_entity(std::move(entity));
    
    // Access through reference
    EXPECT_EQ(ref->id, "test_entity");
    EXPECT_FLOAT_EQ(ref->position.x, 10.0f);
    EXPECT_FLOAT_EQ(ref->position.y, 20.0f);
}

// Test npc::Drive creation
TEST(DriveTest, CreateDrive) {
    // Create a npc::Drive with type and intensity
    npc::Drive hunger(npc::drive::Sustenance{}, 75.0f);
    
    // Check fields
    EXPECT_FLOAT_EQ(hunger.intensity, 75.0f);
    EXPECT_TRUE(std::holds_alternative<npc::drive::Sustenance>(hunger.type));
}

// Test NPC creation
TEST(NPCTest, CreateNPC) {
    // Create components
    entity::Entity entity("test_npc",  world::Position(15.0f, 25.0f));
    auto entity_ref = entity::Entity::storage::make_entity(std::move(entity));
    
    npc::NPCIdentity identity(entity_ref);
    auto identity_ref = npc::NPCIdentity::storage::make_entity(std::move(identity));
    
    std::vector<npc::Drive> drives = {
        npc::Drive(npc::drive::Sustenance{}, 50.0f),
        npc::Drive(npc::drive::Curiosity{}, 60.0f)
    };
    
    memory::PerceptionBuffer buffer({});
    auto perception = memory::PerceptionBuffer::storage::make_entity(std::move(buffer));
    
    // Create NPC
    npc::NPC npc(
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
    EXPECT_TRUE(std::holds_alternative<npc::drive::Sustenance>(npc.drives[0].type));
    EXPECT_TRUE(std::holds_alternative<npc::drive::Curiosity>(npc.drives[1].type));
    EXPECT_FLOAT_EQ(npc.drives[0].intensity, 50.0f);
    EXPECT_FLOAT_EQ(npc.drives[1].intensity, 60.0f);
    EXPECT_TRUE(npc.perception->recent_perceptions.empty());
    EXPECT_TRUE(npc.episodic_memory.empty());
}