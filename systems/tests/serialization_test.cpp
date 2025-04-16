#include <gtest/gtest.h>
#include <filesystem>
#include <fstream>
#include <nlohmann/json.hpp>
#include <history_game/systems/utility/serialization.h>

using namespace history_game::systems;
using json = nlohmann::json;

// Test path for serialization output
const std::string TEST_OUTPUT_PATH = "test_output";
const std::string TEST_LOG_FILE = TEST_OUTPUT_PATH + "/test_events.json";

class SerializationTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create test output directory if it doesn't exist
        std::filesystem::create_directories(TEST_OUTPUT_PATH);
        
        // Remove any existing test file
        if (std::filesystem::exists(TEST_LOG_FILE)) {
            std::filesystem::remove(TEST_LOG_FILE);
        }
    }
    
    void TearDown() override {
        // Clean up test files
        if (std::filesystem::exists(TEST_LOG_FILE)) {
            std::filesystem::remove(TEST_LOG_FILE);
        }
    }
    
    // Helper to read the test log file into a JSON object
    json readLogFile() {
        std::ifstream file(TEST_LOG_FILE);
        EXPECT_TRUE(file.is_open());
        
        json data;
        file >> data;
        return data;
    }
};

// Test logger initialization
TEST_F(SerializationTest, LoggerInitialization) {
    utility::SimulationLogger logger;
    
    // Test initialization
    EXPECT_TRUE(logger.initialize(TEST_LOG_FILE));
    EXPECT_TRUE(logger.isInitialized());
    EXPECT_EQ(logger.getOutputPath(), TEST_LOG_FILE);
    
    // Test shutdown
    logger.shutdown();
    EXPECT_FALSE(logger.isInitialized());
}

// Test tick events serialization
TEST_F(SerializationTest, TickEvents) {
    utility::SimulationLogger logger;
    EXPECT_TRUE(logger.initialize(TEST_LOG_FILE));
    
    // Current time
    uint64_t current_time = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    
    // Log a tick start event
    auto tick_start_event = utility::createTickStartEvent(
        current_time, 
        1, // tick
        1  // generation
    );
    logger.logEvent(tick_start_event);
    
    // Shutdown logger to write to file
    logger.shutdown();
    
    // Read the log file
    json log_data = readLogFile();
    
    // Check that it's an array with 1 event
    EXPECT_TRUE(log_data.is_array());
    EXPECT_EQ(log_data.size(), 1);
    
    // Check event (tick start)
    EXPECT_EQ(log_data[0]["type"], utility::event_type::TickStart{}.name);
    EXPECT_EQ(log_data[0]["tick_number"], 1);
    EXPECT_EQ(log_data[0]["generation"], 1);
}