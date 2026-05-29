#pragma once

#include <string>

const std::string create_user_query =
  "INSERT INTO users (user_name, password_hash) " 
  "VALUES ($1, $2) RETURNING "
  "user_id";

const std::string find_user_by_name_query = 
  "SELECT user_id "
  "FROM users "
  "WHERE user_name = $1";

const std::string get_password_hash_query =
  "SELECT password_hash "
  "FROM users "
  "WHERE user_name = $1";

const std::string find_personal_chat_query = 
  "SELECT c.chat_id "
  "FROM chats c "
  "JOIN chat_members cm1 ON c.chat_id = cm1.chat_id AND cm1.user_id = $1 "
  "JOIN chat_members cm2 ON c.chat_id = cm2.chat_id AND cm2.user_id = $2 "
  "WHERE c.is_group = false";

const std::string create_personal_chat_query = 
  "INSERT INTO chats (is_group) "
  "VALUES (false) "
  "RETURNING chat_id";

const std::string add_user_to_chat_query = 
  "INSERT INTO chat_members (chat_id, user_id) "
  "VALUES ($1, $2)";

const std::string create_group_chat_query = 
  "INSERT INTO chats (is_group, chat_name) "
  "VALUES (true, $1) "
  "RETURNING chat_id";

const std::string remove_user_from_chat_query = 
  "DELETE FROM chat_members "
  "WHERE chat_id = $1 AND user_id = $2";

const std::string check_is_group_query = 
  "SELECT is_group "
  "FROM chats "
  "WHERE chat_id = $1";

const std::string send_message_query =
  "INSERT INTO messages (chat_id, sender_id, text_message) "
  "VALUES ($1, $2, $3) "
  "RETURNING message_id, sent_time";

const std::string check_membership_query =
  "SELECT 1 "
  "FROM chat_members "
  "WHERE chat_id = $1 AND user_id = $2";

const std::string get_messages_query = 
  "SELECT m.message_id, m.sender_id, u.user_name, m.text_message, m.sent_time "
  "FROM messages m "
  "JOIN users u ON m.sender_id = u.user_id "
  "WHERE m.chat_id = $1 "
  "ORDER BY m.sent_time ASC "
  "LIMIT $2 "
  "OFFSET $3 ";

const std::string get_chats_for_user_query = 
  "SELECT c.chat_id, c.is_group, "
  "  CASE WHEN NOT c.is_group THEN "
  "    (SELECT u.user_name FROM chat_members cm "
  "     JOIN users u ON cm.user_id = u.user_id "
  "     WHERE cm.chat_id = c.chat_id AND cm.user_id != $1 LIMIT 1) "
  "  ELSE c.chat_name END AS display_name, "
  "  (SELECT text_message FROM messages WHERE chat_id = c.chat_id ORDER BY sent_time DESC LIMIT 1) AS last_message, "
  "  (SELECT sent_time FROM messages WHERE chat_id = c.chat_id ORDER BY sent_time DESC LIMIT 1) AS last_time "
  "FROM chats c "
  "JOIN chat_members cm ON c.chat_id = cm.chat_id "
  "WHERE cm.user_id = $1 "
  "ORDER BY last_time DESC NULLS LAST";

const std::string edit_message_query = 
  "UPDATE messages "
  "SET text_message = $1 "
  "WHERE message_id = $2 AND sender_id = $3 "
  "RETURNING message_id";

const std::string delete_message_query =
  "DELETE "
  "FROM messages "
  "WHERE message_id = $1 AND sender_id = $2 "
  "RETURNING message_id";
