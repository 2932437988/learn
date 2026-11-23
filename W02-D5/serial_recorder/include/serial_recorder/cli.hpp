#pragma once
#include <iosfwd>
#include <optional>
#include <string>

namespace recorder {

struct Options {
    std::string command;
    std::string device = "-";
    std::string file;
    std::size_t capacity = 256;
    double speed = 1.0;
};

std::optional<Options> parse_cli(int argc, char** argv, std::ostream& err);

}