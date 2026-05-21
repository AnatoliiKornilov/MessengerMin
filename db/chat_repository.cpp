#include "chat_repository.hpp"
#include "db_queries.hpp"

ChatRepository::ChatRepository(DataBase& db) : db_(db) {}

std::string ChatRepository::create_personal_chat(const std::string& user_id_1, const std::string& user_id_2) {
  pqxx::work transaction{db_.connection()};

  pqxx::result existing = transaction.exec_params(find_personal_chat_query, user_id_1, user_id_2);

  if (!existing.empty()) {
    transaction.commit();

    std::string chat_id = existing[0][0].as<std::string>();

    return chat_id;
  }

  pqxx::row new_chat = transaction.exec_params1(create_personal_chat_query);
  std::string chat_id = new_chat[0].as<std::string>();

  transaction.exec_params(add_user_to_chat_query, chat_id, user_id_1);
  transaction.exec_params(add_user_to_chat_query, chat_id, user_id_2);

  transaction.commit();

  return chat_id;
}

std::string ChatRepository::create_group(const std::string& creator_user_id, const std::string& group_name) {
  pqxx::work transaction{db_.connection()};

  pqxx::row new_chat = transaction.exec_params1(create_group_chat_query, group_name);
  std::string chat_id = new_chat[0].as<std::string>();

  transaction.exec_params(add_user_to_chat_query, chat_id, creator_user_id);

  transaction.commit();

  return chat_id;
}

void ChatRepository::add_member(const std::string& chat_id, const std::string& user_id) {
  pqxx::work transaction{db_.connection()};

  if (!is_group_chat(transaction, chat_id)) {
    transaction.commit();
    throw std::runtime_error("Cannot add member to non-group chat");
  }

  try {
    transaction.exec_params(add_user_to_chat_query, chat_id, user_id);
  } catch (const pqxx::unique_violation& e) {
    throw std::runtime_error("User already in chat");
  } catch (const std::exception& e) {
    throw std::runtime_error(std::string("Database error: ") + e.what());
  }

  transaction.commit();
}

void ChatRepository::remove_member(const std::string& chat_id, const std::string& user_id) {
  pqxx::work transaction{db_.connection()};

  if (!is_group_chat(transaction, chat_id)) {
    transaction.commit();
    throw std::runtime_error("Cannot remove member from non-group chat");
  }

  pqxx::result res = transaction.exec_params(remove_user_from_chat_query, chat_id, user_id);
  
  if (res.affected_rows() == 0) {
    throw std::runtime_error("User not in chat");
  }

  transaction.commit();
}

bool ChatRepository::is_group_chat(pqxx::work& transaction, const std::string& chat_id) {
  pqxx::row row = transaction.exec_params1(check_is_group_query, chat_id);

  return row[0].as<bool>();
}
