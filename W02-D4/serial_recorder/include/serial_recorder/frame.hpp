#pragma once
#include <cstddef>
#include <cstdint>
#include <optional>
#include <vector>

namespace recorder {

constexpr std::uint8_t kFrameHeader = 0xAA;
constexpr std::uint8_t kFrameTail = 0x55;

struct Frame {
    std::uint64_t ts_us = 0;
    std::vector<std::uint8_t> payload;
};

std::vector<std::uint8_t> encode_frame(const Frame& f);

class FrameDecoder {
    public:
    std::optional<Frame> feed(std::uint8_t b);
    std::size_t bad_frames() const { return bad_frames_; }

    private:
    enum class State {Header, Length, Payload, Checksum, Tail };
    State state_ = State::Header;
    std::uint8_t len_ = 0;
    std::uint8_t checksum_ = 0;
    std::vector<std::uint8_t> payload_;
    std::size_t bad_frames_ = 0;

    void go_header_with(std::uint8_t b) {
        state_ = (b == kFrameHeader) ? State::Length : State::Header;
    }
};

}