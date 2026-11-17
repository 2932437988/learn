#include <robot_template/config.hpp>

#include <gtest/gtest.h>

#include <cstdio>
#include <fstream>
#include <stdexcept>
#include <string>

namespace {
const char* kTmp = "rt_test_config_tmp.json";

void write_config(const std::string& text) {
  std::ofstream out(kTmp);
  out << text;
}
} // namespace

TEST(ConfigTest, LoadsFieldsAndDefaults) {
  write_config(R"({"capacity": 16, "items": 32, "log_level": "debug"})");
  const auto cfg = robot_template::load_config(kTmp);
  EXPECT_EQ(cfg.capacity, 16u);
  EXPECT_EQ(cfg.items, 32u);
  EXPECT_EQ(cfg.log_level, "debug");
  std::remove(kTmp);

  write_config(R"({"capacity": 4})");
  const auto partial = robot_template::load_config(kTmp);
  EXPECT_EQ(partial.capacity, 4u);
  EXPECT_EQ(partial.items, 1000u); // 未给出的字段保留默认值
  EXPECT_EQ(partial.log_level, "info");
  std::remove(kTmp);
}

TEST(ConfigTest, RejectsBadInput) {
  EXPECT_THROW(robot_template::load_config("no_such_file.json"), std::runtime_error);

  write_config(R"({"capacity": 0})");
  EXPECT_THROW(robot_template::load_config(kTmp), std::runtime_error);
  std::remove(kTmp);

  write_config("{ this is not json");
  EXPECT_ANY_THROW(robot_template::load_config(kTmp));
  std::remove(kTmp);
}