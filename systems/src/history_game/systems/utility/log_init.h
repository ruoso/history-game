#ifndef HISTORY_GAME_SYSTEMS_UTILITY_LOG_INIT_H
#define HISTORY_GAME_SYSTEMS_UTILITY_LOG_INIT_H

#include <string>
#include <optional>
#include <vector>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>

namespace history_game {
namespace log_init {

/**
 * Initialize spdlog with simulation settings
 */
inline void initialize(
  const std::string& console_level = "info",
  const std::optional<std::string>& file_path = std::nullopt,
  const std::string& file_level = "trace"
) {
  // Set the console log level
  auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
  console_sink->set_level(spdlog::level::from_str(console_level));
  
  // Create sinks vector
  std::vector<spdlog::sink_ptr> sinks{console_sink};
  
  // Add file sink if path provided
  if (file_path) {
    auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(file_path.value(), true);
    file_sink->set_level(spdlog::level::from_str(file_level));
    sinks.push_back(file_sink);
  }
  
  // Create the logger with all sinks
  auto simulation_logger = std::make_shared<spdlog::logger>("simulation", sinks.begin(), sinks.end());
  simulation_logger->set_level(spdlog::level::trace);
  simulation_logger->flush_on(spdlog::level::info);
  
  // Register the logger
  spdlog::register_logger(simulation_logger);
  
  // Set as default logger
  spdlog::set_default_logger(simulation_logger);
  
  // Set pattern
  spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%n] [%^%l%$] %v");
  
  spdlog::info("Logging initialized");
}

/**
 * Shutdown and flush all loggers
 */
inline void shutdown() {
  spdlog::info("Logging shutdown");
  spdlog::shutdown();
}

} // namespace log_init
} // namespace history_game

#endif // HISTORY_GAME_LOG_INIT_H