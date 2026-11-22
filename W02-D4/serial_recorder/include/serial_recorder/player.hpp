#pragma once
#include <istream>
#include <ostream>

namespace recorder {

struct PlayerOptions {
    double speed = 1.0;
};

std::size_t run_player(std::istream& in, std::ostream& out, const PlayerOptions& opt);

}