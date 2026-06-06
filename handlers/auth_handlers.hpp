#pragma once

#include "../src/app_context.hpp"

#include "httplib.h"

void handle_register(const httplib::Request& request, 
                    httplib::Response& response, 
                    AppContext& context);

void handle_login(const httplib::Request& request, 
                    httplib::Response& response, 
                    AppContext& context); 

void handle_refresh(const httplib::Request& request, 
                    httplib::Response& response, 
                    AppContext& context);
