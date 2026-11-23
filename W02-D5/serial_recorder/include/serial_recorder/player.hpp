#pragma once
#include <cstdint>
#include <istream>
#include <ostream>

namespace recorder {

struct PlayerOptions {
    double speed = 1.0;
};

// 纯函数：把录制相对时间按倍速换算成回放的微秒偏移。
// 抽出来是为了能精确单测，测试里不必 sleep 真实时间。
std::int64_t playback_offset_us(std::uint64_t ts_us, std::uint64_t base_us, double speed);

std::size_t run_player(std::istream& in, std::ostream& out, const PlayerOptions& opt);

}