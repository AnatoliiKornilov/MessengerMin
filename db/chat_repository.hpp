#pragma once

#include "database.hpp"

#include <pqxx/pqxx>

#include <string>

class ChatRepository {
 public:

  explicit ChatRepository(DataBase& db);

  std::string create_personal_chat(const std::string& user_id_1, const std::string& user_id_2);

  std::string create_group(const std::string& creator_user_id, const std::string& group_name);

  void add_member(const std::string& chat_id, const std::string& user_id);

  void remove_member(const std::string& chat_id, const std::string& user_id);

 private:

  bool is_group_chat(pqxx::work& transaction, const std::string& chat_id);

  DataBase& db_;
};
