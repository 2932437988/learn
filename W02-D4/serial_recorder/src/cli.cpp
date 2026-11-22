#include "serial_recorder/cli.hpp"

#include <getopt.h>
#include <cstdlib>
#include <iostream>

namespace recorder {

void print_usage(std::ostream& os) {
  os << "Usage:\n"
     << "  serial_recorder record --out FILE [--device PATH] [--capacity N]\n"
     << "  serial_recorder play   --in FILE  [--device PATH] [--speed X]\n"
     << "\n"
     << "  --device -   use stdin (record) / stdout (play)\n"
     << "  examples:\n"
     << "    python3 tools/fake_serial.py | serial_recorder record --device - --out rec.jsonl\n"
     << "    serial_recorder play --in rec.jsonl --speed 2 --device - > out.bin\n";
}

std::optional<Options> parse_cli(int argc, char** argv, std::ostream& err) {
  if (argc < 2) {
    err << "error: missing subcommand (record|play)\n";
    print_usage(err);
    return std::nullopt;
  }

  Options opt;
  opt.command = argv[1];
  if (opt.command != "record" && opt.command != "play") {
    err << "error: unknown subcommand '" << opt.command << "'\n";
    print_usage(err);
    return std::nullopt;
  }

  static struct option longopts[] = {
      {"device",   required_argument, nullptr, 'd'},
      {"out",      required_argument, nullptr, 'o'},
      {"in",       required_argument, nullptr, 'i'},
      {"capacity", required_argument, nullptr, 'c'},
      {"speed",    required_argument, nullptr, 's'},
      {"help",     no_argument,       nullptr, 'h'},
      {nullptr, 0, nullptr, 0},
  };

  optind = 2;                              // 跳过 argv[1] 的子命令
  int c;
  while ((c = getopt_long(argc, argv, "d:o:i:c:s:h", longopts, nullptr)) != -1) {
    switch (c) {
      case 'd': opt.device = optarg; break;
      case 'o': opt.file = optarg; break;
      case 'i': opt.file = optarg; break;
      case 'c': opt.capacity = std::strtoul(optarg, nullptr, 10); break;
      case 's': opt.speed = std::strtod(optarg, nullptr); break;
      case 'h': print_usage(std::cout); return std::nullopt;  // help 不算错误
      case '?':
      default:
        print_usage(err);
        return std::nullopt;
    }
  }

  if (opt.file.empty()) {
    err << "error: " << (opt.command == "record" ? "--out" : "--in") << " is required\n";
    print_usage(err);
    return std::nullopt;
  }
  if (opt.command == "record" && opt.capacity == 0) {
    err << "error: --capacity must be > 0\n";
    return std::nullopt;
  }
  if (opt.command == "play" && opt.speed <= 0.0) {
    err << "error: --speed must be > 0\n";
    return std::nullopt;
  }
  return opt;
}

}  // namespace recorder