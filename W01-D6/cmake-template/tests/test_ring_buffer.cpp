#include <robot_template/ring_buffer.hpp>

#include <gtest/gtest.h>

#include <chrono>
#include <future>
#include <optional>
#include <thread>

using namespace std::chrono_literals;

// 1) 基础 FIFO 与容量/大小（对应 D4 6.3 第一组）
TEST(RingBufferTest, BasicFifo) {
  RingBuffer<int> q(3);
  EXPECT_EQ(q.capacity(), 3u);
  EXPECT_EQ(q.size(), 0u);
  for (int i = 0; i < 3; ++i)
    EXPECT_TRUE(q.push(i));
  EXPECT_EQ(q.size(), 3u);
  for (int i = 0; i < 3; ++i) {
    auto v = q.pop();
    ASSERT_TRUE(v.has_value());
    EXPECT_EQ(*v, i);
  }
  EXPECT_EQ(q.size(), 0u);
}

// 2) 满时 push 阻塞，取走一个后放行（对应第二组）
TEST(RingBufferTest, PushBlocksWhenFull) {
  RingBuffer<int> q(2);
  ASSERT_TRUE(q.push(1));
  ASSERT_TRUE(q.push(2));

  auto fut = std::async(std::launch::async, [&] { return q.push(3); });
  EXPECT_EQ(fut.wait_for(50ms), std::future_status::timeout);

  auto v = q.pop();
  ASSERT_TRUE(v.has_value());
  EXPECT_EQ(*v, 1);
  EXPECT_EQ(fut.wait_for(2s), std::future_status::ready);
  EXPECT_TRUE(fut.get());
  EXPECT_EQ(q.size(), 2u);
}

// 3) close 后：push 返回 false，取完已有元素后 pop 返回 nullopt（第三组）
TEST(RingBufferTest, CloseSemantics) {
  RingBuffer<int> q(2);
  ASSERT_TRUE(q.push(10));
  ASSERT_TRUE(q.push(20));
  q.close();
  EXPECT_FALSE(q.push(30));
  auto a = q.pop();
  ASSERT_TRUE(a.has_value());
  EXPECT_EQ(*a, 10);
  auto b = q.pop();
  ASSERT_TRUE(b.has_value());
  EXPECT_EQ(*b, 20);
  EXPECT_FALSE(q.pop().has_value());
}

// 4) 空时 pop 阻塞，close 唤醒；单元素也能被取到（第四组）
TEST(RingBufferTest, PopBlocksThenCloseWakes) {
  RingBuffer<int> q(2);
  auto fut = std::async(std::launch::async, [&] { return q.pop(); });
  EXPECT_EQ(fut.wait_for(50ms), std::future_status::timeout);

  ASSERT_TRUE(q.push(7));
  EXPECT_EQ(fut.wait_for(2s), std::future_status::ready);
  auto v = fut.get();
  ASSERT_TRUE(v.has_value());
  EXPECT_EQ(*v, 7);

  auto pending = std::async(std::launch::async, [&] { return q.pop(); });
  EXPECT_EQ(pending.wait_for(50ms), std::future_status::timeout);
  q.close(); // notify_all 唤醒所有等待者
  EXPECT_EQ(pending.wait_for(2s), std::future_status::ready);
  EXPECT_FALSE(pending.get().has_value());
}

// 5) SPSC 并发求和，用 long long 防溢出（第五组）
TEST(RingBufferTest, SpscConcurrentSum) {
  constexpr int n = 100000;
  RingBuffer<long long> q(64);

  std::thread producer([&] {
    for (int i = 0; i < n; ++i)
      q.push(i);
    q.close();
  });
  long long sum = 0;
  std::thread consumer([&] {
    while (auto v = q.pop())
      sum += *v;
  });
  producer.join();
  consumer.join();

  EXPECT_EQ(sum, static_cast<long long>(n) * (n - 1) / 2);
  EXPECT_FALSE(q.pop().has_value());
}