#pragma once

#include <pqxx/pqxx>

#include <memory>
#include <string>

class DataBase {
 public:

  explicit DataBase(const std::string& connection_string)
      : conn_(std::make_unique<pqxx::connection>(connection_string)) {}

  inline pqxx::connection& connection() {
    return *conn_;
  }

 private:

  std::unique_ptr<pqxx::connection> conn_;
};
