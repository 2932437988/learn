#include "serial_recorder/frame.hpp"
#include <gtest/gtest.h>

using namespace recorder;

static std::vector<std::uint8_t> wire(const std::vector<std::uint8_t>& payload) {
  Frame f;
  f.payload = payload;
  return encode_frame(f);
}

TEST(Frame, RoundTrip) {
  auto bytes = wire({0x10, 0x20, 0x30});
  FrameDecoder d;
  std::optional<Frame> got;
  for (auto b : bytes) got = d.feed(b);
  ASSERT_TRUE(got.has_value());
  EXPECT_EQ(got->payload, (std::vector<std::uint8_t>{0x10, 0x20, 0x30}));
  EXPECT_EQ(d.bad_frames(), 0u);
}

TEST(Frame, TwoFramesInOneRead) {          // 粘包
  auto a = wire({1, 2});
  auto b = wire({3, 4, 5});
  a.insert(a.end(), b.begin(), b.end());
  FrameDecoder d;
  int count = 0;
  for (auto x : a) if (d.feed(x)) ++count;
  EXPECT_EQ(count, 2);
}

TEST(Frame, SplitAcrossReads) {            // 半包
  auto bytes = wire({9, 8, 7, 6});
  FrameDecoder d;
  std::optional<Frame> got;
  for (std::size_t i = 0; i < bytes.size(); ++i) {
    auto r = d.feed(bytes[i]);
    if (r) got = r;
  }
  ASSERT_TRUE(got.has_value());
  EXPECT_EQ(got->payload, (std::vector<std::uint8_t>{9, 8, 7, 6}));
}

TEST(Frame, BadChecksumCounted) {          // 坏帧
  auto bytes = wire({1, 2, 3});
  bytes[bytes.size() - 2] ^= 0xFF;         // 改坏校验字节
  FrameDecoder d;
  int count = 0;
  for (auto x : bytes) if (d.feed(x)) ++count;
  EXPECT_EQ(count, 0);
  EXPECT_EQ(d.bad_frames(), 1u);
}