#include "robot_template/config.hpp"

#include <fstream>
#include <nlohmann/json.hpp>
#include <stdexcept>

namespace robot_template {

AppConfig load_config(const std::string& path) {
  std::ifstream in(path);
  if (!in)
    throw std::runtime_error("cannot open config file: " + path);

  nlohmann::json j;
  in >> j; // 解析失败抛 nlohmann::json::parse_error

  AppConfig cfg;
  cfg.capacity = j.value("capacity", cfg.capacity);
  cfg.items = j.value("items", cfg.items);
  cfg.log_level = j.value("log_level", cfg.log_level);
  if (cfg.capacity == 0)
    throw std::runtime_error("capacity must be > 0");
  return cfg;
}

} // namespace robot_template