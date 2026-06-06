#pragma once

#include "app_context.hpp"

#include "httplib.h"

void setup_routes(httplib::Server& server, AppContext& context);

void setup_auth_routes(httplib::Server& server, AppContext& context);

void setup_chat_routes(httplib::Server& server, AppContext& context);

void setup_message_routes(httplib::Server& server, AppContext& context);
