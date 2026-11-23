#pragma once
#include <cstddef>
#include <istream>
#include <string>

namespace recorder {

struct RecorderOptions {
    std::size_t capacity = 256;
    std::string log_path = "recording.jsonl";
};

std::size_t run_recorder(std::istream& in, const RecorderOptions& opt);

}