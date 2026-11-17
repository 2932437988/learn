#include "robot_template/log.hpp"

#include <spdlog/spdlog.h>

namespace robot_template {

void init_logging(const std::string& level) {
  if (level == "debug")
    spdlog::set_level(spdlog::level::debug);
  else
    spdlog::set_level(spdlog::level::info);
}

void log_info(const std::string& message) {
  spdlog::info("{}", message);
}

} // namespace robot_template