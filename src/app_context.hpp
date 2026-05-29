#pragma once

#include <string>

#include "../db/chat_repository.hpp"
#include "../db/database.hpp"
#include "../db/message_repository.hpp"
#include "../db/user_repository.hpp"
#include "config.hpp"

struct AppContext {
  AppContext(const Config& config)
      : db(config.db_connection_string, config.db_pool_size),
        users(db),
        chats(db),
        messages(db),
        jwt_secret(config.jwt_secret) {}

  DataBase db;

  UserRepository users;
  ChatRepository chats;
  MessageRepository messages;

  std::string jwt_secret;
};
