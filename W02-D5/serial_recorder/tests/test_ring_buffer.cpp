#include "serial_recorder/ring_buffer.hpp"

#include <gtest/gtest.h>

#include <atomic>
#include <chrono>
#include <cstdint>
#include <optional>
#include <thread>

using namespace std::chrono_literals;

// 观察窗口：给被测线程 50ms 证明"它确实阻塞了"
TEST(RingBuffer, EmptyPopBlocksUntilClose) {
  RingBuffer<int> q(4);
  std::atomic<bool> returned{false};
  std::optional<int> result;

  std::thread t([&] {
    result = q.pop();
    returned.store(true);
  });

  std::this_thread::sleep_for(50ms);
  EXPECT_FALSE(returned.load()) << "empty pop should block";

  q.close();                       // 断连：唤醒阻塞的消费者
  t.join();
  EXPECT_TRUE(returned.load());
  EXPECT_FALSE(result.has_value()) << "drained empty queue returns nullopt";
}

TEST(RingBuffer, FullPushBlocksUntilClose) {
  RingBuffer<int> q(1);
  ASSERT_TRUE(q.push(10));         // 唯一槽位占满

  std::atomic<bool> returned{false};
  bool accepted = true;
  std::thread t([&] {
    accepted = q.push(20);
    returned.store(true);
  });

  std::this_thread::sleep_for(50ms);
  EXPECT_FALSE(returned.load()) << "full push should block (backpressure)";

  q.close();
  t.join();
  EXPECT_FALSE(accepted) << "push after close must be rejected";
}

TEST(RingBuffer, CloseDrainsRemaining) {
  RingBuffer<int> q(4);
  q.push(1);
  q.push(2);
  q.push(3);
  q.close();                       // 关闭不等于丢弃

  EXPECT_EQ(q.pop().value(), 1);
  EXPECT_EQ(q.pop().value(), 2);
  EXPECT_EQ(q.pop().value(), 3);
  EXPECT_FALSE(q.pop().has_value()) << "nullopt only after drained";
}

TEST(RingBuffer, FifoOrderPreserved) {
  RingBuffer<int> q(3);
  q.push(10);
  q.push(20);
  q.push(30);

  EXPECT_EQ(q.pop().value(), 10);  // 若满时覆盖最旧，这里会错位
  EXPECT_EQ(q.pop().value(), 20);
  EXPECT_EQ(q.pop().value(), 30);
}

TEST(RingBuffer, SpscNoLossNoDup) {
  constexpr int kN = 10000;
  RingBuffer<int> q(64);

  std::thread producer([&] {
    for (int i = 0; i < kN; ++i) {
      q.push(i);                   // 满则阻塞，等消费者取走
    }
    q.close();                     // 生产完毕，通知消费者排空退出
  });

  int count = 0;
  std::int64_t sum = 0;
  while (auto v = q.pop()) {
    ++count;
    sum += *v;
  }
  producer.join();

  EXPECT_EQ(count, kN);
  EXPECT_EQ(sum, static_cast<std::int64_t>(kN) * (kN - 1) / 2);
}