#include "serial_recorder/player.hpp"
#include "serial_recorder/frame.hpp"

#include <chrono>
#include <nlohmann/json.hpp>
#include <string>
#include <thread>
#include <vector>

namespace recorder {

using json = nlohmann::json;
using clock = std::chrono::steady_clock;

std::size_t run_player(std::istream& in, std::ostream& out, const PlayerOptions& opt) {
    std::vector<Frame> frames;
    std::string line;
    while (std::getline(in, line)) {
        if (line.empty()) continue;
        json j = json::parse(line);
        Frame f;
        f.ts_us = j.at("ts_us").get<std::uint64_t>();
        f.payload = j.at("payload").get<std::vector<std::uint8_t>>();
        frames.push_back(std::move(f));
    }
    if (frames.empty()) return 0;

    const std::uint64_t base = frames.front().ts_us;
    const auto t0 = clock::now();

    for (const Frame& f : frames) {
        const auto offset_us = static_cast<std::int64_t>(static_cast<double>(f.ts_us - base) / opt.speed);
        const auto target = t0 + std::chrono::microseconds(offset_us);
        std::this_thread::sleep_until(target);

        const auto bytes = encode_frame(f);
        out.write(reinterpret_cast<const char*>(bytes.data()),static_cast<std::streamsize>(bytes.size()));
    }
    out.flush();
    return frames.size();
}

}