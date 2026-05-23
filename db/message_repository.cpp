#include "message_repository.hpp"

#include <stdexcept>

#include "db_queries.hpp"

MessageRepository::MessageRepository(DataBase& db) : db_(db) {}

std::pair<std::string, std::string> MessageRepository::send_message(
    const std::string& chat_id, 
    const std::string& sender_id,
    const std::string& text) {
  pqxx::work transaction{db_.connection()};

  if (!check_membership(transaction, chat_id, sender_id)) {
    throw std::runtime_error("User is not a member of this chat");
  }

  pqxx::row row =
      transaction.exec_params1(send_message_query, chat_id, sender_id, text);

  std::string message_id = row[0].as<std::string>();
  std::string sent_at = row[1].as<std::string>();

  transaction.commit();

  return {message_id, sent_at};
}

std::vector<MessageData> MessageRepository::get_chat_messages(
    const std::string& chat_id, 
    unsigned int limit, 
    unsigned int offset) {
  pqxx::work transaction{db_.connection()};

  pqxx::result rows =
      transaction.exec_params(get_messages_query, chat_id, limit, offset);

  transaction.commit();

  std::vector<MessageData> messages;

  for (const pqxx::row& row : rows) {
    messages.emplace_back(
      row[0].as<std::string>(), 
      row[1].as<std::string>(),      
      row[2].as<std::string>(), 
      row[3].as<std::string>(),             
      row[4].as<std::string>()
    );
  }

  return messages;
}

bool MessageRepository::check_membership(
    pqxx::work& transaction, 
    const std::string& chat_id, 
    const std::string& user_id) {
  pqxx::result result = transaction.exec_params(check_membership_query, chat_id, user_id);

  return !result.empty();
}
