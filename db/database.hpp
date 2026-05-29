#pragma once

#include "connection_pool.hpp"

#include <pqxx/pqxx>

#include <memory>
#include <string>

class DataBase {
 public:

  explicit DataBase(const std::string& connection_string, std::size_t pool_size = 100)
      : pool_(std::make_unique<ConnectionPool>(connection_string, pool_size)) {}

  inline ConnectionPool::ConnectionGuard connection() {
    return pool_->connection();
  }

 private:

  std::unique_ptr<ConnectionPool> pool_;
};
