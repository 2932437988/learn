#pragma once
#include <cstddef>
#include <string>

namespace robot_template {

struct AppConfig {
  std::size_t capacity = 8;
  std::size_t items = 1000;
  std::string log_level = "info";
};

// 读取 JSON 配置；文件打不开或字段非法时抛 std::runtime_error
AppConfig load_config(const std::string& path);

} // namespace robot_template