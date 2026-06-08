#pragma once

#include <chrono>
#include <deque>
#include <mutex>
#include <string>
#include <unordered_map>

class RateLimiter {
 public:
  using Clock = std::chrono::steady_clock;

  bool is_allowed(
      const std::string& ip, 
      size_t max_requests = 200,
      std::chrono::seconds window = std::chrono::seconds(60)) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto& history = requests_[ip];

    auto now = Clock::now();

    while (!history.empty() && (now - history.front()) > window) {
      history.pop_front();
    }

    if (history.size() >= max_requests) {
      return false;
    }

    history.push_back(now);

    if (++call_counter_ > cleanup_interval_) {
      cleanup_expired(window);
      call_counter_ = 0;
    }

    return true;
  }

 private:
  void cleanup_expired(std::chrono::seconds window) {
    auto now = Clock::now();

    for (auto it = requests_.begin(); it != requests_.end();) {
      auto& q = it->second;

      while (!q.empty() && (now - q.front()) > window) {
        q.pop_front();
      }

      if (q.empty()) {
        it = requests_.erase(it);
      } else {
        ++it;
      }
    }
  }

  std::unordered_map<std::string, std::deque<Clock::time_point>> requests_;

  std::mutex mutex_;
  
  std::size_t call_counter_ = 0;
  static const std::size_t cleanup_interval_ = 1'000'000;
};
