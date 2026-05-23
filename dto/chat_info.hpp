#pragma once

#include <string>

struct ChatInfo {
  std::string chat_id;
  bool is_group;
  std::string name;
  std::string last_message;
  std::string last_time;
};
