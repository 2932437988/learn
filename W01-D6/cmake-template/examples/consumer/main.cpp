#include <robot_template/config.hpp>

#include <iostream>

int main() {
  const robot_template::AppConfig cfg;
  std::cout << "consumer ok, default capacity = " << cfg.capacity << '\n';
  return 0;
}