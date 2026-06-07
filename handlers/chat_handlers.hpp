#pragma once

#include "../src/app_context.hpp"

#include "httplib.h"

#include <string>

void handle_create_personal_chat(const httplib::Request& request,
                                httplib::Response& response,
                                AppContext& context);

void handle_create_group(const httplib::Request& request,
                        httplib::Response& response,
                        AppContext& context);

void handle_get_chats(const httplib::Request& request,
                      httplib::Response& response,
                      AppContext& context);

void handle_add_member(const httplib::Request& request,
                      httplib::Response& response,
                      AppContext& context,
                      const std::string& chat_id);

void handle_remove_member(const httplib::Request& request,
                          httplib::Response& response,
                          AppContext& context,
                          const std::string& chat_id,
                          const std::string& member_id);

void handle_find_user(const httplib::Request& request,
                      httplib::Response& response,
                      AppContext& context);
