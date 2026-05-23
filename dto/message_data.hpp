#pragma once

#include <string>

struct MessageData {
  std::string message_id;
  std::string sender_id;
  std::string sender_name;
  std::string text;
  std::string sent_at;
};
