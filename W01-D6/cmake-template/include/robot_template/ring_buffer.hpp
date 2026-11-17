#pragma once
#include <condition_variable>
#include <cstddef>
#include <mutex>
#include <optional>
#include <stdexcept>
#include <utility>
#include <vector>

template <typename T> class RingBuffer {
public:
  explicit RingBuffer(std::size_t capacity) : buf_(capacity), cap_(capacity) {
    if (capacity == 0)
      throw std::invalid_argument("RingBuffer capacity must be > 0");
  }

  RingBuffer(const RingBuffer&) = delete;
  RingBuffer& operator=(const RingBuffer&) = delete;

  bool push(T value) {
    std::unique_lock<std::mutex> lk(m_);
    not_full_.wait(lk, [this] { return size_ < cap_ || closed_; });
    if (closed_)
      return false;
    buf_[tail_] = std::move(value);
    tail_ = (tail_ + 1) % cap_;
    ++size_;
    lk.unlock();
    not_empty_.notify_one();
    return true;
  }

  std::optional<T> pop() {
    std::unique_lock<std::mutex> lk(m_);
    not_empty_.wait(lk, [this] { return size_ > 0 || closed_; });
    if (size_ == 0)
      return std::nullopt;
    T value = std::move(buf_[head_]);
    head_ = (head_ + 1) % cap_;
    --size_;
    lk.unlock();
    not_full_.notify_one();
    return std::optional<T>{std::move(value)};
  }

  // 追加到 public 区（片段，不是完整文件）
  bool try_push(T value) {
    std::lock_guard<std::mutex> lk(m_);
    if (closed_ || size_ == cap_)
      return false;
    buf_[tail_] = std::move(value);
    tail_ = (tail_ + 1) % cap_;
    ++size_;
    // 这里仍在持锁，notify 放在锁内是可接受的简化；追求性能可改成解锁后通知
    not_empty_.notify_one();
    return true;
  }

  std::optional<T> try_pop() {
    std::lock_guard<std::mutex> lk(m_);
    if (size_ == 0)
      return std::nullopt;
    T value = std::move(buf_[head_]);
    head_ = (head_ + 1) % cap_;
    --size_;
    not_full_.notify_one();
    return std::optional<T>{std::move(value)};
  }

  void close() {
    {
      std::lock_guard<std::mutex> lk(m_);
      closed_ = true;
    }
    not_empty_.notify_all();
    not_full_.notify_all();
  }

  std::size_t size() const {
    std::lock_guard<std::mutex> lk(m_);
    return size_;
  }

  std::size_t capacity() const {
    return cap_;
  }

private:
  mutable std::mutex m_;
  std::condition_variable not_full_, not_empty_;
  std::vector<T> buf_;
  const std::size_t cap_;
  std::size_t head_ = 0, tail_ = 0, size_ = 0;
  bool closed_ = false;
};