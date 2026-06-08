#include "message_handlers.hpp"

#include "../src/middleware/auth_middleware.hpp"

#include <nlohmann/json.hpp>

using JSON = nlohmann::json;

void handle_send_message(
    const httplib::Request& request, 
    httplib::Response& response,
    AppContext& context, 
    const std::string& chat_id) {
  std::string token = extract_token(request);

  std::optional<std::string> user_id = extract_user_id_from_token(token, context.jwt_secret);

  if (!user_id.has_value()) {
    response.status = 401;
    response.set_content(R"({"error":"Invalid token"})", "application/json");
    return;
  }

  try {
    JSON body = nlohmann::json::parse(request.body);

    std::string text = body.at("text");

    auto [message_id, sent_time] = context.messages.send_message(chat_id, *user_id, text);

    response.status = 201;

    JSON response_json = {{"message_id", message_id}, {"sent_at", sent_time}};

    response.set_content(response_json.dump(), "application/json");
  } catch (const std::runtime_error& exception) {
    response.status = 403;

    JSON response_json = {{"error", exception.what()}};

    response.set_content(response_json.dump(), "application/json");
  } catch (const std::exception& exception) {
    response.status = 400;

    JSON response_json = {{"error", exception.what()}};

    response.set_content(response_json.dump(), "application/json");
  }
}

void handle_get_messages(
    const httplib::Request& request, 
    httplib::Response& response,
    AppContext& context, 
    const std::string& chat_id) {
  std::string token = extract_token(request);

  std::optional<std::string> user_id = extract_user_id_from_token(token, context.jwt_secret);

  if (!user_id.has_value()) {
    response.status = 401;
    response.set_content(R"({"error":"Invalid token"})", "application/json");
    return;
  }

  if (!context.chats.is_member(chat_id, *user_id)) {
    response.status = 403;
    response.set_content(R"({"error":"Access denied"})", "application/json");
    return;
  }

  int limit = 50;
  int offset = 0;

  if (request.has_param("limit")) { 
    limit = std::stoi(request.get_param_value("limit")); 
  }

  if (request.has_param("offset")) {
    offset = std::stoi(request.get_param_value("offset"));
  }

  try {
    std::vector<MessageData> messages = context.messages.get_chat_messages(chat_id, limit, offset);

    JSON arr = nlohmann::json::array();

    for (const MessageData& message : messages) {
      JSON object;

      object["message_id"] = message.message_id;
      object["sender_id"] = message.sender_id;
      object["sender_name"] = message.sender_name;
      object["text"] = message.text;
      object["sent_at"] = message.sent_at;

      arr.push_back(object);
    }

    response.status = 200;
    response.set_content(arr.dump(), "application/json");
  } catch (const std::exception& exception) {
    response.status = 400;

    JSON response_json = {{"error", exception.what()}};

    response.set_content(response_json.dump(), "application/json");
  }
}

void handle_edit_message(
    const httplib::Request& request, 
    httplib::Response& response,
    AppContext& context, 
    const std::string& message_id) {
  std::string token = extract_token(request);

  std::optional<std::string> user_id = extract_user_id_from_token(token, context.jwt_secret);

  if (!user_id.has_value()) {
    response.status = 401;
    response.set_content(R"({"error":"Invalid token"})", "application/json");
    return;
  }

  try {
    JSON body = nlohmann::json::parse(request.body);

    std::string new_text = body.at("text");

    context.messages.edit_message(message_id, *user_id, new_text);

    response.status = 200;
    response.set_content(R"({"status":"ok"})", "application/json");
  } catch (const std::runtime_error& exception) {
    response.status = 403;

    JSON response_json = {{"error", exception.what()}};

    response.set_content(response_json.dump(), "application/json");
  } catch (const std::exception& exception) {
    response.status = 400;

    JSON response_json = {{"error", exception.what()}};

    response.set_content(response_json.dump(), "application/json");
  }
}

void handle_delete_message(
    const httplib::Request& request, 
    httplib::Response& response,
    AppContext& context, 
    const std::string& message_id) {
  std::string token = extract_token(request);

  std::optional<std::string> user_id = extract_user_id_from_token(token, context.jwt_secret);

  if (!user_id.has_value()) {
    response.status = 401;
    response.set_content(R"({"error":"Invalid token"})", "application/json");
    return;
  }

  try {
    context.messages.delete_message(message_id, *user_id);

    response.status = 200;
    response.set_content(R"({"status":"ok"})", "application/json");
  } catch (const std::runtime_error& exception) {
    response.status = 403;

    JSON response_json = {{"error", exception.what()}};

    response.set_content(response_json.dump(), "application/json");
  } catch (const std::exception& exception) {
    response.status = 400;

    JSON response_json = {{"error", exception.what()}};

    response.set_content(response_json.dump(), "application/json");
  }
}
