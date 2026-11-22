#pragma once
#include <condition_variable>
#include <cstddef>
#include <mutex>
#include <optional>
#include <utility>
#include <vector>

template <typename T>
class RingBuffer {
 public:
  explicit RingBuffer(std::size_t capacity) : buf_(capacity), cap_(capacity) {}
  RingBuffer(const RingBuffer&) = delete;
  RingBuffer& operator=(const RingBuffer&) = delete;

  bool push(T value) {                       // 满则阻塞；close 后返回 false
    std::unique_lock<std::mutex> lk(m_);
    not_full_.wait(lk, [this] { return size_ < cap_ || closed_; });
    if (closed_) return false;
    buf_[tail_] = std::move(value);
    tail_ = (tail_ + 1) % cap_;
    ++size_;
    lk.unlock();
    not_empty_.notify_one();
    return true;
  }

  std::optional<T> pop() {                   // 空则阻塞；close 后排空再 nullopt
    std::unique_lock<std::mutex> lk(m_);
    not_empty_.wait(lk, [this] { return size_ > 0 || closed_; });
    if (size_ == 0) return std::nullopt;
    T value = std::move(buf_[head_]);
    head_ = (head_ + 1) % cap_;
    --size_;
    lk.unlock();
    not_full_.notify_one();
    return std::optional<T>{std::move(value)};
  }

  void close() {
    { std::lock_guard<std::mutex> lk(m_); closed_ = true; }
    not_full_.notify_all();
    not_empty_.notify_all();
  }

 private:
  std::vector<T> buf_;
  std::size_t cap_, head_ = 0, tail_ = 0, size_ = 0;
  bool closed_ = false;
  std::mutex m_;
  std::condition_variable not_full_, not_empty_;
};