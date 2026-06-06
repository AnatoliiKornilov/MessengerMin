#pragma once

#include "../src/app_context.hpp"

#include "httplib.h"

#include <string>

void handle_send_message(const httplib::Request& request, 
                        httplib::Response& response, 
                        AppContext& context, 
                        const std::string& chat_id);


void handle_get_messages(const httplib::Request& request, 
                        httplib::Response& response, 
                        AppContext& context, 
                        const std::string& chat_id);


void handle_edit_message(const httplib::Request& request,
                        httplib::Response& response, 
                        AppContext& context, 
                        const std::string& message_id);


void handle_delete_message(const httplib::Request& request,
                          httplib::Response& response, 
                          AppContext& context, 
                          const std::string& message_id);
