#include <iostream>
#include <string>
#include <vector>
#include <cpioo/managed_entity.hpp>
#include <gtest/gtest.h>

// Simple test to verify we can use inside-out-objects
TEST(InsideOutTest, CanCreate) {
  // Create a simple entity storage type
  using TestStorage = cpioo::managed_entity::storage<std::string, 10, uint32_t>;
  
  // Create a value
  std::string value = "test value";
  auto ref = TestStorage::make_entity(std::move(value));
  
  // Verify we can access it through the -> operator
  EXPECT_EQ(ref.operator->()->compare("test value"), 0);
}

int main(int argc, char** argv) {
  std::cout << "Testing inside-out-objects library..." << std::endl;
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}