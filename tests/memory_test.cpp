#include <gtest/gtest.h>
#include "memory/memory_entry.h"
#include "memory/memory_episode.h"
#include "memory/perception_buffer.h"
#include "memory/memory_system.h"
#include "memory/episode_formation.h"
#include "entity/entity.h"
#include "npc/npc.h"
#include "npc/npc_identity.h"
#include "action/action_type.h"
#include "action/action_sequence.h"

using namespace history_game;

// Test memory entry creation
TEST(MemoryTest, CreateMemoryEntry) {
    // Create entity and identity
    Entity entity("test_entity", Position(10.0f, 20.0f));
    auto entity_ref = Entity::storage::make_entity(std::move(entity));
    
    NPCIdentity identity(entity_ref);
    auto identity_ref = NPCIdentity::storage::make_entity(std::move(identity));
    
    // Create memory entry
    MemoryEntry entry(
        100,  // timestamp
        identity_ref,  // actor
        action_type::Move{},  // action
        entity_ref  // target entity
    );
    
    // Check fields
    EXPECT_EQ(entry.timestamp, 100);
    EXPECT_EQ(entry.actor, identity_ref);
    EXPECT_TRUE(std::holds_alternative<action_type::Move>(entry.action));
    EXPECT_TRUE(entry.target_entity.has_value());
    EXPECT_EQ(entry.target_entity.value(), entity_ref);
    EXPECT_FALSE(entry.target_object.has_value());
}

// Test perception buffer
TEST(MemoryTest, PerceptionBuffer) {
    // Create some memory entries
    Entity entity("test_entity", Position(10.0f, 20.0f));
    auto entity_ref = Entity::storage::make_entity(std::move(entity));
    
    NPCIdentity identity(entity_ref);
    auto identity_ref = NPCIdentity::storage::make_entity(std::move(identity));
    
    MemoryEntry entry1(100, identity_ref, action_type::Move{}, entity_ref);
    auto entry1_ref = MemoryEntry::storage::make_entity(std::move(entry1));
    
    MemoryEntry entry2(110, identity_ref, action_type::Observe{}, entity_ref);
    auto entry2_ref = MemoryEntry::storage::make_entity(std::move(entry2));
    
    // Create perception buffer
    std::vector<MemoryEntry::ref_type> entries = { entry1_ref, entry2_ref };
    PerceptionBuffer buffer(entries);
    
    // Check buffer
    EXPECT_EQ(buffer.recent_perceptions.size(), 2);
    EXPECT_EQ(buffer.recent_perceptions[0], entry1_ref);
    EXPECT_EQ(buffer.recent_perceptions[1], entry2_ref);
}

// Test action sequence creation
TEST(MemoryTest, ActionSequence) {
    // Create memory entries
    Entity entity("test_entity", Position(10.0f, 20.0f));
    auto entity_ref = Entity::storage::make_entity(std::move(entity));
    
    NPCIdentity identity(entity_ref);
    auto identity_ref = NPCIdentity::storage::make_entity(std::move(identity));
    
    MemoryEntry entry1(100, identity_ref, action_type::Move{}, entity_ref);
    auto entry1_ref = MemoryEntry::storage::make_entity(std::move(entry1));
    
    MemoryEntry entry2(110, identity_ref, action_type::Observe{}, entity_ref);
    auto entry2_ref = MemoryEntry::storage::make_entity(std::move(entry2));
    
    // Create action steps
    std::vector<ActionStep> steps = {
        ActionStep(entry1_ref, 0),  // First step (no delay)
        ActionStep(entry2_ref, 10)  // Second step (10 ticks after first)
    };
    
    // Create action sequence
    ActionSequence sequence("test_sequence", steps);
    
    // Check sequence
    EXPECT_EQ(sequence.id, "test_sequence");
    EXPECT_EQ(sequence.steps.size(), 2);
    EXPECT_EQ(sequence.steps[0].memory, entry1_ref);
    EXPECT_EQ(sequence.steps[0].delay_after_previous, 0);
    EXPECT_EQ(sequence.steps[1].memory, entry2_ref);
    EXPECT_EQ(sequence.steps[1].delay_after_previous, 10);
}

// Test memory episode creation
TEST(MemoryTest, MemoryEpisode) {
    // Create action sequence
    Entity entity("test_entity", Position(10.0f, 20.0f));
    auto entity_ref = Entity::storage::make_entity(std::move(entity));
    
    NPCIdentity identity(entity_ref);
    auto identity_ref = NPCIdentity::storage::make_entity(std::move(identity));
    
    MemoryEntry entry1(100, identity_ref, action_type::Move{}, entity_ref);
    auto entry1_ref = MemoryEntry::storage::make_entity(std::move(entry1));
    
    std::vector<ActionStep> steps = { ActionStep(entry1_ref, 0) };
    ActionSequence sequence("test_sequence", steps);
    auto sequence_ref = ActionSequence::storage::make_entity(std::move(sequence));
    
    // Create impacts
    std::vector<Drive> impacts = {
        Drive(drive::Curiosity{}, -0.5f)  // Satisfied curiosity
    };
    
    // Create memory episode
    MemoryEpisode episode(
        100,  // start time
        110,  // end time
        sequence_ref,
        impacts,
        1  // first occurrence
    );
    
    // Check episode
    EXPECT_EQ(episode.start_time, 100);
    EXPECT_EQ(episode.end_time, 110);
    EXPECT_EQ(episode.action_sequence, sequence_ref);
    EXPECT_EQ(episode.drive_impacts.size(), 1);
    EXPECT_TRUE(std::holds_alternative<drive::Curiosity>(episode.drive_impacts[0].type));
    EXPECT_FLOAT_EQ(episode.drive_impacts[0].intensity, -0.5f);
    EXPECT_EQ(episode.repetition_count, 1);
}

// Test perception buffer update
TEST(MemorySystemTest, UpdatePerceptionBuffer) {
    // Create initial buffer
    Entity entity("test_entity", Position(10.0f, 20.0f));
    auto entity_ref = Entity::storage::make_entity(std::move(entity));
    
    NPCIdentity identity(entity_ref);
    auto identity_ref = NPCIdentity::storage::make_entity(std::move(identity));
    
    MemoryEntry entry1(100, identity_ref, action_type::Move{}, entity_ref);
    auto entry1_ref = MemoryEntry::storage::make_entity(std::move(entry1));
    
    std::vector<MemoryEntry::ref_type> initial_entries = { entry1_ref };
    PerceptionBuffer buffer(initial_entries);
    auto buffer_ref = PerceptionBuffer::storage::make_entity(std::move(buffer));
    
    // Create new entries
    MemoryEntry entry2(110, identity_ref, action_type::Observe{}, entity_ref);
    auto entry2_ref = MemoryEntry::storage::make_entity(std::move(entry2));
    
    std::vector<MemoryEntry::ref_type> new_entries = { entry2_ref };
    
    // Update buffer
    auto updated_buffer = memory_system::updatePerceptionBuffer(buffer_ref, new_entries);
    
    // Check updated buffer
    EXPECT_EQ(updated_buffer->recent_perceptions.size(), 2);
    EXPECT_EQ(updated_buffer->recent_perceptions[0], entry1_ref);
    EXPECT_EQ(updated_buffer->recent_perceptions[1], entry2_ref);
}