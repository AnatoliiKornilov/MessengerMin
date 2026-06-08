#include "app.hpp"
#include "rate_limiter.hpp"

#include "../handlers/auth_handlers.hpp"
#include "../handlers/chat_handlers.hpp"
#include "../handlers/message_handlers.hpp"

void setup_auth_routes(httplib::Server& server, AppContext& context) {
  server.Post("/api/auth/register", [&](const httplib::Request& request, httplib::Response& response) {
    handle_register(request, response, context);
  });

  server.Post("/api/auth/login", [&](const httplib::Request& request, httplib::Response& response) {
    handle_login(request, response, context);
  });

  server.Post("/api/auth/refresh", [&](const httplib::Request& request, httplib::Response& response) {
    handle_refresh(request, response, context);
  });
}

void setup_chat_routes(httplib::Server& server, AppContext& context) {
  server.Post("/api/chats", [&](const httplib::Request& request, httplib::Response& response) {
    handle_create_personal_chat(request, response, context);
  });

  server.Post("/api/chats/group", [&](const httplib::Request& request, httplib::Response& response) {
    handle_create_group(request, response, context);
  });

  server.Get("/api/chats", [&](const httplib::Request& request, httplib::Response& response) {
    handle_get_chats(request, response, context);
  });

  server.Post(R"(/api/chats/([^/]+)/members)", [&](const httplib::Request& request, httplib::Response& response) {
    std::string chat_id = request.matches[1];
    handle_add_member(request, response, context, chat_id);
  });

  server.Delete(R"(/api/chats/([^/]+)/members/([^/]+))", [&](const httplib::Request& request, httplib::Response& response) {
    std::string chat_id = request.matches[1];
    std::string member_id = request.matches[2];
    handle_remove_member(request, response, context, chat_id, member_id);
  });

  server.Get("/api/users/by-name", [&](const httplib::Request& request, httplib::Response& response) {
    handle_find_user(request, response, context);
  });
}

void setup_message_routes(httplib::Server& server, AppContext& context) {
  server.Post(R"(/api/chats/([^/]+)/messages)", [&](const httplib::Request& request, httplib::Response& response) {
    std::string chat_id = request.matches[1];
    handle_send_message(request, response, context, chat_id);
  });

  server.Get(R"(/api/chats/([^/]+)/messages)", [&](const httplib::Request& request, httplib::Response& response) {
    std::string chat_id = request.matches[1];
    handle_get_messages(request, response, context, chat_id);
  });

  server.Put(R"(/api/messages/([^/]+))", [&](const httplib::Request& request, httplib::Response& response) {
    std::string message_id = request.matches[1];
    handle_edit_message(request, response, context, message_id);
  });

  server.Delete(R"(/api/messages/([^/]+))", [&](const httplib::Request& request, httplib::Response& response) {
    std::string message_id = request.matches[1];
    handle_delete_message(request, response, context, message_id);
  });
}

void setup_routes(httplib::Server& server, AppContext& context) {
  static RateLimiter limiter;

  server.set_pre_routing_handler([](const httplib::Request& request, httplib::Response& response) {
    response.set_header("Access-Control-Allow-Origin", "*");

    response.set_header("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");

    response.set_header("Access-Control-Allow-Headers", "Content-Type, Authorization");

    if (request.method == "OPTIONS") {
      response.status = 204;
      return httplib::Server::HandlerResponse::Handled;
    }

    std::string ip = request.remote_addr;

    if (!limiter.is_allowed(ip)) {
      response.status = 429;

      response.set_content("{\"error\":\"Too many requests\"}", "application/json");
      
      return httplib::Server::HandlerResponse::Handled;
    }

    return httplib::Server::HandlerResponse::Unhandled;
  });

  setup_auth_routes(server, context);
  setup_chat_routes(server, context);
  setup_message_routes(server, context);
}
