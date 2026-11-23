#include "serial_recorder/frame.hpp"
#include <utility>

namespace recorder {

std::vector<std::uint8_t> encode_frame(const Frame& f) {
    std::vector<std::uint8_t> out;
    out.reserve(f.payload.size() + 5);
    out.push_back(kFrameHeader);
    out.push_back(static_cast<std::uint8_t>(f.payload.size()));
    std::uint8_t xor_sum = 0;
    for (std::uint8_t b : f.payload) {
        out.push_back(b);
        xor_sum ^= b;
    }
    out.push_back(xor_sum);
    out.push_back(kFrameTail);
    return out;
}

std::optional<Frame> FrameDecoder::feed(std::uint8_t b) {
    switch (state_) {
        case State::Header:
            if (b == kFrameHeader) state_ = State::Length;
            break;
        case State::Length:
            len_ = b;
            payload_.clear();
            checksum_ = 0;
            state_ = (len_ == 0) ? State::Checksum : State::Payload;
            break;
        case State::Payload:
            payload_.push_back(b);
            checksum_ ^= b;
            if (payload_.size() >= len_) state_ = State::Checksum;
            break;
        case State::Checksum:
            if (b != checksum_) { ++bad_frames_; go_header_with(b); }
            else state_ = State::Tail;
            break;
        case State::Tail:
            if (b == kFrameTail) {
                Frame f;
                f.payload = std::move(payload_);
                state_ = State::Header;
                return f;
            }
            ++bad_frames_;
            go_header_with(b);
            break;
    }
    return std::nullopt;
}

}