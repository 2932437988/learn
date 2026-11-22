#include "serial_recorder/cli.hpp"
#include "serial_recorder/player.hpp"
#include "serial_recorder/recorder.hpp"

#include <fstream>
#include <iostream>

int main(int argc, char** argv) {
  auto opt = recorder::parse_cli(argc, argv, std::cerr);
  if (!opt) return 2;

  try {
    if (opt->command == "record") {
      if (opt->device == "-") {
        recorder::RecorderOptions ropt;
        ropt.capacity = opt->capacity;
        ropt.log_path = opt->file;
        auto n = recorder::run_recorder(std::cin, ropt);
        std::cerr << "recorded " << n << " frames\n";
      } else {
        std::ifstream in(opt->device, std::ios::binary);
        if (!in) { std::cerr << "error: cannot open " << opt->device << "\n"; return 1; }
        recorder::RecorderOptions ropt;
        ropt.capacity = opt->capacity;
        ropt.log_path = opt->file;
        auto n = recorder::run_recorder(in, ropt);
        std::cerr << "recorded " << n << " frames\n";
      }
    } else {  // play
      std::ifstream in(opt->file);
      if (!in) { std::cerr << "error: cannot open " << opt->file << "\n"; return 1; }
      recorder::PlayerOptions popt;
      popt.speed = opt->speed;
      if (opt->device == "-") {
        auto n = recorder::run_player(in, std::cout, popt);
        std::cerr << "played " << n << " frames\n";
      } else {
        std::ofstream out(opt->device, std::ios::binary);
        if (!out) { std::cerr << "error: cannot open " << opt->device << "\n"; return 1; }
        auto n = recorder::run_player(in, out, popt);
        std::cerr << "played " << n << " frames\n";
      }
    }
  } catch (const std::exception& e) {
    std::cerr << "error: " << e.what() << "\n";
    return 1;
  }
  return 0;
}