#pragma once
#include <string>

namespace robot_template {

void init_logging(const std::string& level); // "debug" / "info"，其它值按 info 处理
void log_info(const std::string& message);

} // namespace robot_template