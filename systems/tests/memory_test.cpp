#include <gtest/gtest.h>
#include <history_game/datamodel/memory/memory_entry.h>
#include <history_game/datamodel/memory/memory_episode.h>
#include <history_game/datamodel/memory/perception_buffer.h>
#include <history_game/systems/memory/memory_system.h>
#include <history_game/systems/memory/episode_formation.h>
#include <history_game/datamodel/entity/entity.h>
#include <history_game/datamodel/npc/npc.h>
#include <history_game/datamodel/npc/npc_identity.h>
#include <history_game/datamodel/action/action_type.h>
#include <history_game/datamodel/action/action_sequence.h>
#include <history_game/datamodel/world/position.h>

// Don't use "using namespace" for the ambiguous namespaces

// Test memory entry creation
TEST(MemoryTest, CreateMemoryEntry) {
    // Create entity and identity
    history_game::datamodel::entity::Entity entity("test_entity", history_game::datamodel::world::Position(10.0f, 20.0f));
    auto entity_ref = history_game::datamodel::entity::Entity::storage::make_entity(std::move(entity));
    
    history_game::datamodel::npc::NPCIdentity identity(entity_ref);
    auto identity_ref = history_game::datamodel::npc::NPCIdentity::storage::make_entity(std::move(identity));
    
    // Create memory entry
    history_game::datamodel::memory::MemoryEntry entry(
        100,  // timestamp
        identity_ref,  // actor
        history_game::datamodel::action::action_type::Move{},  // action
        entity_ref  // target entity
    );
    
    // Check fields
    EXPECT_EQ(entry.timestamp, 100);
    EXPECT_EQ(entry.actor, identity_ref);
    EXPECT_TRUE(std::holds_alternative<history_game::datamodel::action::action_type::Move>(entry.action));
    EXPECT_TRUE(entry.target_entity.has_value());
    EXPECT_EQ(entry.target_entity.value(), entity_ref);
    EXPECT_FALSE(entry.target_object.has_value());
}

// Test perception buffer
TEST(MemoryTest, PerceptionBuffer) {
    // Create some memory entries
    history_game::datamodel::entity::Entity entity("test_entity", history_game::datamodel::world::Position(10.0f, 20.0f));
    auto entity_ref = history_game::datamodel::entity::Entity::storage::make_entity(std::move(entity));
    
    history_game::datamodel::npc::NPCIdentity identity(entity_ref);
    auto identity_ref = history_game::datamodel::npc::NPCIdentity::storage::make_entity(std::move(identity));
    
    history_game::datamodel::memory::MemoryEntry entry1(100, identity_ref, history_game::datamodel::action::action_type::Move{}, entity_ref);
    auto entry1_ref = history_game::datamodel::memory::MemoryEntry::storage::make_entity(std::move(entry1));
    
    history_game::datamodel::memory::MemoryEntry entry2(110, identity_ref, history_game::datamodel::action::action_type::Observe{}, entity_ref);
    auto entry2_ref = history_game::datamodel::memory::MemoryEntry::storage::make_entity(std::move(entry2));
    
    // Create perception buffer
    std::vector<history_game::datamodel::memory::MemoryEntry::ref_type> entries = { entry1_ref, entry2_ref };
    history_game::datamodel::memory::PerceptionBuffer buffer(entries);
    
    // Check buffer
    EXPECT_EQ(buffer.recent_perceptions.size(), 2);
    EXPECT_EQ(buffer.recent_perceptions[0], entry1_ref);
    EXPECT_EQ(buffer.recent_perceptions[1], entry2_ref);
}

// Test action sequence creation
TEST(MemoryTest, ActionSequence) {
    // Create memory entries
    history_game::datamodel::entity::Entity entity("test_entity", history_game::datamodel::world::Position(10.0f, 20.0f));
    auto entity_ref = history_game::datamodel::entity::Entity::storage::make_entity(std::move(entity));
    
    history_game::datamodel::npc::NPCIdentity identity(entity_ref);
    auto identity_ref = history_game::datamodel::npc::NPCIdentity::storage::make_entity(std::move(identity));
    
    history_game::datamodel::memory::MemoryEntry entry1(100, identity_ref, history_game::datamodel::action::action_type::Move{}, entity_ref);
    auto entry1_ref = history_game::datamodel::memory::MemoryEntry::storage::make_entity(std::move(entry1));
    
    history_game::datamodel::memory::MemoryEntry entry2(110, identity_ref, history_game::datamodel::action::action_type::Observe{}, entity_ref);
    auto entry2_ref = history_game::datamodel::memory::MemoryEntry::storage::make_entity(std::move(entry2));
    
    // Create action steps
    std::vector<history_game::datamodel::action::ActionStep> steps = {
        history_game::datamodel::action::ActionStep(entry1_ref, 0),  // First step (no delay)
        history_game::datamodel::action::ActionStep(entry2_ref, 10)  // Second step (10 ticks after first)
    };
    
    // Create action sequence
    history_game::datamodel::action::ActionSequence sequence("test_sequence", steps);
    
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
    history_game::datamodel::entity::Entity entity("test_entity", history_game::datamodel::world::Position(10.0f, 20.0f));
    auto entity_ref = history_game::datamodel::entity::Entity::storage::make_entity(std::move(entity));
    
    history_game::datamodel::npc::NPCIdentity identity(entity_ref);
    auto identity_ref = history_game::datamodel::npc::NPCIdentity::storage::make_entity(std::move(identity));
    
    history_game::datamodel::memory::MemoryEntry entry1(100, identity_ref, history_game::datamodel::action::action_type::Move{}, entity_ref);
    auto entry1_ref = history_game::datamodel::memory::MemoryEntry::storage::make_entity(std::move(entry1));
    
    std::vector<history_game::datamodel::action::ActionStep> steps = { history_game::datamodel::action::ActionStep(entry1_ref, 0) };
    history_game::datamodel::action::ActionSequence sequence("test_sequence", steps);
    auto sequence_ref = history_game::datamodel::action::ActionSequence::storage::make_entity(std::move(sequence));
    
    // Create impacts
    std::vector<history_game::datamodel::npc::Drive> impacts = {
        history_game::datamodel::npc::Drive(history_game::datamodel::npc::drive::Curiosity{}, -0.5f)  // Satisfied curiosity
    };
    
    // Create memory episode
    history_game::datamodel::memory::MemoryEpisode episode(
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
    EXPECT_TRUE(std::holds_alternative<history_game::datamodel::npc::drive::Curiosity>(episode.drive_impacts[0].type));
    EXPECT_FLOAT_EQ(episode.drive_impacts[0].intensity, -0.5f);
    EXPECT_EQ(episode.repetition_count, 1);
}

// Test perception buffer update
TEST(MemorySystemTest, UpdatePerceptionBuffer) {
    // Create initial buffer
    history_game::datamodel::entity::Entity entity("test_entity", history_game::datamodel::world::Position(10.0f, 20.0f));
    auto entity_ref = history_game::datamodel::entity::Entity::storage::make_entity(std::move(entity));
    
    history_game::datamodel::npc::NPCIdentity identity(entity_ref);
    auto identity_ref = history_game::datamodel::npc::NPCIdentity::storage::make_entity(std::move(identity));
    
    history_game::datamodel::memory::MemoryEntry entry1(100, identity_ref, history_game::datamodel::action::action_type::Move{}, entity_ref);
    auto entry1_ref = history_game::datamodel::memory::MemoryEntry::storage::make_entity(std::move(entry1));
    
    std::vector<history_game::datamodel::memory::MemoryEntry::ref_type> initial_entries = { entry1_ref };
    history_game::datamodel::memory::PerceptionBuffer buffer(initial_entries);
    auto buffer_ref = history_game::datamodel::memory::PerceptionBuffer::storage::make_entity(std::move(buffer));
    
    // Create new entries
    history_game::datamodel::memory::MemoryEntry entry2(110, identity_ref, history_game::datamodel::action::action_type::Observe{}, entity_ref);
    auto entry2_ref = history_game::datamodel::memory::MemoryEntry::storage::make_entity(std::move(entry2));
    
    std::vector<history_game::datamodel::memory::MemoryEntry::ref_type> new_entries = { entry2_ref };
    
    // Update buffer
    auto updated_buffer = history_game::systems::memory::updatePerceptionBuffer(buffer_ref, new_entries);
    
    // Check updated buffer
    EXPECT_EQ(updated_buffer->recent_perceptions.size(), 2);
    EXPECT_EQ(updated_buffer->recent_perceptions[0], entry1_ref);
    EXPECT_EQ(updated_buffer->recent_perceptions[1], entry2_ref);
}