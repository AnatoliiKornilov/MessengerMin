#pragma once

#include <cstdlib>
#include <stdexcept>
#include <string>

struct Config {
  static Config from_env() {
    Config config;

    const char* db_connection = std::getenv("DBCONN");
    if (db_connection == nullptr) {
      throw std::runtime_error("DB_CONN not set");
    }
    config.db_connection_string = db_connection;

    const char* jwt = std::getenv("JWT_SECRET");
    if (jwt == nullptr) {
      throw std::runtime_error("JWT_SECRET not set");
    }
    config.jwt_secret = jwt;

    const char* port = std::getenv("PORT");
    if (port != nullptr) {
      config.port = std::stoi(port);
    }

    const char* pool = std::getenv("DB_POOL_SIZE");
    if (pool != nullptr) {
      config.db_pool_size = std::stoul(pool);
    }

    return config;
  }

  std::string db_connection_string;
  std::string jwt_secret;
  int port = 8080;
  std::size_t db_pool_size = 100;
};
