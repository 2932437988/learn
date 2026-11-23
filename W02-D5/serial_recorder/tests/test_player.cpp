#include "serial_recorder/frame.hpp"
#include "serial_recorder/player.hpp"

#include <gtest/gtest.h>

#include <cstdint>
#include <nlohmann/json.hpp>
#include <sstream>
#include <string>
#include <vector>

using namespace recorder;

// —— 倍速公式：纯函数，精确断言，不依赖时钟 ——

TEST(PlaybackOffset, SpeedOneKeepsInterval) {
  EXPECT_EQ(playback_offset_us(100000, 0, 1.0), 100000);
  EXPECT_EQ(playback_offset_us(200000, 100000, 1.0), 100000);
}

TEST(PlaybackOffset, SpeedTwoHalvesInterval) {
  EXPECT_EQ(playback_offset_us(100000, 0, 2.0), 50000);
  EXPECT_EQ(playback_offset_us(200000, 0, 2.0), 100000);
}

TEST(PlaybackOffset, SpeedHalfDoublesInterval) {
  EXPECT_EQ(playback_offset_us(100000, 0, 0.5), 200000);
}

TEST(PlaybackOffset, BaseAlignedFirstFrameIsZero) {
  EXPECT_EQ(playback_offset_us(500, 500, 2.0), 0);
  EXPECT_EQ(playback_offset_us(500, 600, 2.0), 0);  // 倒序输入，防御分支
}

// —— 端到端：JSONL 解析 + 编码，用 stringstream 不碰真实设备 ——

TEST(Player, JsonlRoundTripToBytes) {
  nlohmann::json j1;
  j1["ts_us"] = 0;
  j1["payload"] = std::vector<std::uint8_t>{1, 2};
  nlohmann::json j2;
  j2["ts_us"] = 100000;
  j2["payload"] = std::vector<std::uint8_t>{3};

  std::istringstream in(j1.dump() + "\n" + j2.dump() + "\n");
  std::ostringstream out;
  PlayerOptions opt;
  opt.speed = 1000.0;              // 100ms → 0.1ms，让测试瞬时完成
  const auto n = run_player(in, out, opt);
  ASSERT_EQ(n, 2u);

  Frame f1; f1.payload = {1, 2};
  Frame f2; f2.payload = {3};
  const auto b1 = encode_frame(f1);
  const auto b2 = encode_frame(f2);
  std::string expected(reinterpret_cast<const char*>(b1.data()), b1.size());
  expected.append(reinterpret_cast<const char*>(b2.data()), b2.size());
  EXPECT_EQ(out.str(), expected);
}

TEST(Player, EmptyInputReturnsZero) {
  std::istringstream in("");
  std::ostringstream out;
  EXPECT_EQ(run_player(in, out, PlayerOptions{}), 0u);
}