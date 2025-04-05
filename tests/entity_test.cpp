#include <gtest/gtest.h>
#include <memory>
#include "entity/entity.h"
#include "entity/position.h"

using namespace history_game;

TEST(EntityTests, BasicConstructionAndRetrieval) {
    // Create a new entity with a unique ID and position
    Entity entity("test_entity_1", Position(10.0f, 20.0f));
    
    // Create a reference to the entity in storage
    auto ref = Entity::storage::make_entity(std::move(entity));
    
    // Test that we can retrieve the entity's properties
    EXPECT_EQ(ref->id, "test_entity_1");
    EXPECT_FLOAT_EQ(ref->position.x, 10.0f);
    EXPECT_FLOAT_EQ(ref->position.y, 20.0f);
}

TEST(EntityTests, MultipleEntities) {
    // Create multiple entities and store them
    auto entity1 = Entity::storage::make_entity(Entity("entity1", Position(1.0f, 2.0f)));
    auto entity2 = Entity::storage::make_entity(Entity("entity2", Position(3.0f, 4.0f)));
    auto entity3 = Entity::storage::make_entity(Entity("entity3", Position(5.0f, 6.0f)));
    
    // Test they maintain their unique identities
    EXPECT_EQ(entity1->id, "entity1");
    EXPECT_EQ(entity2->id, "entity2");
    EXPECT_EQ(entity3->id, "entity3");
    
    // Test they maintain their positions
    EXPECT_FLOAT_EQ(entity1->position.x, 1.0f);
    EXPECT_FLOAT_EQ(entity2->position.x, 3.0f);
    EXPECT_FLOAT_EQ(entity3->position.x, 5.0f);
}