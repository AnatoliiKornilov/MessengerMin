#pragma once

#include <pqxx/pqxx>
#include <string>
#include <utility>
#include <vector>

#include "../dto/message_data.hpp"
#include "database.hpp"

class MessageRepository {
 public:

  explicit MessageRepository(DataBase& db);

  std::pair<std::string, std::string> send_message(const std::string& chat_id,
                                                   const std::string& sender_id,
                                                   const std::string& text);

  std::vector<MessageData> get_chat_messages(const std::string& chat_id,
                                             unsigned int limit = 50,
                                             unsigned int offset = 0);

 private:

  bool check_membership(pqxx::work& transaction, 
                        const std::string& chat_id,
                        const std::string& user_id);

  DataBase& db_;
};
