#include "serial_recorder/recorder.hpp"
#include "serial_recorder/frame.hpp"
#include "serial_recorder/ring_buffer.hpp"

#include <array>
#include <chrono>
#include <nlohmann/json.hpp>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/spdlog.h>
#include <thread>

namespace recorder {

using json = nlohmann::json;
using clock = std::chrono::steady_clock;

std::size_t run_recorder(std::istream& in, const RecorderOptions& opt)
{
    RingBuffer<Frame> q(opt.capacity);

    auto logger = spdlog::basic_logger_mt("data", opt.log_path, true);
    logger->set_pattern("%v");

    std::size_t recorded = 0;
    std::thread writer([&] {
        while (auto f = q.pop()) {
            json j;
            j["ts_us"] = f->ts_us;
            j["payload"] = f->payload;
            logger->info(j.dump());
        }
    });

    FrameDecoder decoder;
    const auto start = clock::now();
    // 逐字节读：read(buf, 4096) 是"读满才返回"语义，管道里脚本每 100ms 只写 6 字节，
    // 它会一直阻塞攒到 EOF（脚本退出）才一次性交付全部字节 → 所有帧挤在同一时刻，
    // ts_us 全相等。get() 每个字节到达就处理，才能记录真实到达间隔。
    char ch;
    while (in.get(ch)) {
        if (auto f = decoder.feed(static_cast<std::uint8_t>(ch))) {
            f->ts_us = static_cast<std::uint64_t>(
            std::chrono::duration_cast<std::chrono::microseconds>(clock::now() - start).count());
        if (!q.push(*f)) break;                 // close 后被拒，退出
        ++recorded;
        }
    }

    q.close();                                  // 断连边界：唤醒 writer 排空
    writer.join();
    logger->flush();
    spdlog::info("recorded {} frames, {} bad frames", recorded, decoder.bad_frames());
    spdlog::drop("data");
    return recorded;
}

}  // namespace recorder