#include <robot_template/config.hpp>
#include <robot_template/log.hpp>
#include <robot_template/ring_buffer.hpp> // D4 原样搬来，RingBuffer 在全局命名空间

#include <iostream>
#include <string>
#include <thread>

int main(int argc, char **argv) {
  const std::string path = (argc > 1) ? argv[1] : "config.json";
  try {
    const auto cfg = robot_template::load_config(path);
    robot_template::init_logging(cfg.log_level);

    RingBuffer<long long> q(cfg.capacity); // D4 的 SPSC 环形缓冲
    std::thread producer([&] {
      for (std::size_t i = 0; i < cfg.items; ++i)
        q.push(static_cast<long long>(i));
      q.close(); // close 必须 notify_all（D4 Q6）
    });

    long long sum = 0;
    std::thread consumer([&] {
      while (auto v = q.pop())
        sum += *v; // close 且取空后返回 nullopt
    });
    producer.join();
    consumer.join();

    robot_template::log_info("sum = " + std::to_string(sum));
    std::cout << "sum = " << sum << '\n'; // 判据：0+1+...+(items-1)
    return 0;
  } catch (const std::exception &e) {
    std::cerr << "demo failed: " << e.what() << '\n';
    return 1;
  }
}