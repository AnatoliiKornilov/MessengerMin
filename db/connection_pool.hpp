#pragma once

#include <pqxx/pqxx>

#include <condition_variable>
#include <memory>
#include <mutex>
#include <queue>
#include <string>

class ConnectionPool {
 public:

  ConnectionPool(std::string connection_string, std::size_t pool_size) 
      : connection_string_(connection_string), pool_size_(pool_size) {
    for (std::size_t i = 0; i < pool_size_; ++i) {
      queue_.emplace(std::make_unique<pqxx::connection>(connection_string_));
    }
  }

  struct ConnectionGuard {
    ConnectionGuard(ConnectionPool& pool, std::unique_ptr<pqxx::connection> connection)
      : pool(pool), connection(std::move(connection)) {}


    ~ConnectionGuard() {
      if (connection != nullptr) {
        pool.release_connection(std::move(connection));
      }
    }

    ConnectionPool& pool;
    std::unique_ptr<pqxx::connection> connection;
  };

  ConnectionGuard connection() {
    std::unique_lock<std::mutex> lock(mutex_);
    
    while (queue_.empty()) {
      cv_.wait(lock);
    }

    std::unique_ptr<pqxx::connection> connection = std::move(queue_.front());
    queue_.pop();

    lock.unlock();

    return ConnectionGuard(*this, std::move(connection));
  }

 private:

  void release_connection(std::unique_ptr<pqxx::connection> connection) {
    mutex_.lock();

    queue_.push(std::move(connection));

    cv_.notify_one();

    mutex_.unlock();
  }

  std::string connection_string_;
 
  std::size_t pool_size_ = 0;
  std::queue<std::unique_ptr<pqxx::connection>> queue_;

  std::mutex mutex_;
  std::condition_variable cv_;
};
