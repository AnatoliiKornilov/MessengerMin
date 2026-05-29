#pragma once

#include <jwt-cpp/jwt.h>

#include <optional>
#include <string>

std::optional<std::string> extract_user_id_from_token(const std::string& token, 
                                                      const std::string& secret);
