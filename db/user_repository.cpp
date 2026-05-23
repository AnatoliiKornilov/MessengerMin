#include "db_queries.hpp"
#include "user_repository.hpp"

#include <pqxx/pqxx>

UserRepository::UserRepository(DataBase& db) : db_(db) {}

std::string UserRepository::create_user(const std::string& user_name, const std::string& password_hash) {
  std::unique_lock<std::mutex> lock = db_.lock();
  pqxx::work transaction{db_.connection()};

  pqxx::row response = transaction.exec_params1(create_user_query, user_name, password_hash);

  transaction.commit();
  lock.unlock();

  std::string user_id = response[0].as<std::string>();

  return user_id;
}

std::optional<std::string> UserRepository::find_user_by_name(const std::string& user_name) {
  std::unique_lock<std::mutex> lock = db_.lock();
  pqxx::work transaction{db_.connection()};

  pqxx::result result = transaction.exec_params(find_user_by_name_query, user_name);

  transaction.commit();
  lock.unlock();

  if (result.empty()) {
    return std::nullopt;
  }

  std::string user_id = result[0][0].as<std::string>();

  return user_id;
}
