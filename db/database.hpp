#pragma once

#include <pqxx/pqxx>

#include <memory>
#include <mutex>
#include <string>

class DataBase {
 public:

  explicit DataBase(const std::string& connection_string)
      : conn_(std::make_unique<pqxx::connection>(connection_string)) {}

  inline pqxx::connection& connection() {
    return *conn_;
  }

  inline std::unique_lock<std::mutex> lock() {
    return std::unique_lock<std::mutex>(mutex_);
  }

 private:

  std::unique_ptr<pqxx::connection> conn_;
  std::mutex mutex_;
};
