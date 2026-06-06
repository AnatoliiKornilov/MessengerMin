#pragma once

#include "httplib.h"
#include <jwt-cpp/jwt.h>

#include <optional>
#include <string>

std::optional<std::string> extract_user_id_from_token(const std::string& token, 
                                                      const std::string& secret);

std::optional<std::string> extract_refresh_token_from_cookie(const httplib::Request& request);

std::string extract_token(const httplib::Request& request);
