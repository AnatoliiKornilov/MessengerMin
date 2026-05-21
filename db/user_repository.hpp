#pragma once

#include "database.hpp"

#include <optional>
#include <string>

class UserRepository {
 public:

  explicit UserRepository(DataBase& db);

  std::string create_user(const std::string& user_name, const std::string& password_hash);

  std::optional<std::string> find_user_by_name(const std::string& user_name);

 private:

  DataBase& db_;
};
