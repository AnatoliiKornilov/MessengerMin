#include "chat_handlers.hpp"

#include "../src/middleware/auth_middleware.hpp"

#include <nlohmann/json.hpp>

#include <exception>

using JSON = nlohmann::json;

void handle_create_personal_chat(
    const httplib::Request& request,
    httplib::Response& response, 
    AppContext& context) {
  std::string token = extract_token(request);

  std::optional<std::string> user_id = extract_user_id_from_token(token, context.jwt_secret);

  if (!user_id.has_value()) {
    response.status = 401;
    response.set_content(R"({"error":"Invalid token"})", "application/json");
    return;
  }

  try {
    JSON body = nlohmann::json::parse(request.body);

    std::string target_user_name = body.at("user_name");

    std::optional<std::string> target_id = context.users.find_user_by_name(target_user_name);

    if (!target_id.has_value()) {
      response.status = 404;
      response.set_content(R"({"error":"User not found"})", "application/json");
      return;
    }

    std::string chat_id = context.chats.create_personal_chat(*user_id, *target_id);

    response.status = 201;

    JSON response_json = {{"chat_id", chat_id}};

    response.set_content(response_json.dump(), "application/json");
  } catch (const std::exception& exception) {
    response.status = 400;

    JSON response_json = {{"error", exception.what()}};

    response.set_content(response_json.dump(), "application/json");
  }
}

void handle_create_group(
    const httplib::Request& request, 
    httplib::Response& response,
    AppContext& context) {
  std::string token = extract_token(request);

  std::optional<std::string> user_id = extract_user_id_from_token(token, context.jwt_secret);

  if (!user_id.has_value()) {
    response.status = 401;
    response.set_content(R"({"error":"Invalid token"})", "application/json");
    return;
  }

  try {
    JSON body = nlohmann::json::parse(request.body);

    std::string group_name = body.at("name");
    std::string chat_id = context.chats.create_group(*user_id, group_name);

    response.status = 201;

    JSON response_json = {{"chat_id", chat_id}};

    response.set_content(response_json.dump(), "application/json");
  } catch (const std::exception& exception) {
    response.status = 400;

    JSON response_json = {{"error", exception.what()}};

    response.set_content(response_json.dump(), "application/json");
  }
}

void handle_get_chats(
    const httplib::Request& request, 
    httplib::Response& response,
    AppContext& context) {
  std::string token = extract_token(request);

  std::optional<std::string> user_id = extract_user_id_from_token(token, context.jwt_secret);

  if (!user_id.has_value()) {
    response.status = 401;
    response.set_content(R"({"error":"Invalid token"})", "application/json");
    return;
  }

  try {
    std::vector<ChatInfo> chats = context.chats.get_chats_for_user(*user_id);

    JSON arr = nlohmann::json::array();

    for (const ChatInfo& chat : chats) {
      JSON object;

      object["chat_id"] = chat.chat_id;
      object["is_group"] = chat.is_group;
      object["name"] = chat.name;
      object["last_message"] = chat.last_message;
      object["last_time"] = chat.last_time;

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

void handle_add_member(
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

    std::string new_member_user_name = body.at("user_name");

    std::optional<std::string> new_member_id = context.users.find_user_by_name(new_member_user_name);

    if (!new_member_id.has_value()) {
      response.status = 404;
      response.set_content(R"({"error":"User not found"})", "application/json");
      return;
    }

    context.chats.add_member(chat_id, *new_member_id);

    response.status = 200;
    response.set_content(R"({"status":"ok"})", "application/json");
  } catch (const std::exception& exception) {
    response.status = 400;

    JSON response_json = {{"error", exception.what()}};

    response.set_content(response_json.dump(), "application/json");
  }
}

void handle_remove_member(
    const httplib::Request& request, 
    httplib::Response& response,
    AppContext& context, 
    const std::string& chat_id,
    const std::string& member_id) {
  std::string token = extract_token(request);

  std::optional<std::string> user_id = extract_user_id_from_token(token, context.jwt_secret);

  if (!user_id.has_value()) {
    response.status = 401;
    response.set_content(R"({"error":"Invalid token"})", "application/json");
    return;
  }

  try {
    context.chats.remove_member(chat_id, member_id);

    response.status = 200;
    response.set_content(R"({"status":"ok"})", "application/json");
  } catch (const std::exception& exception) {
    response.status = 400;

    JSON response_json = {{"error", exception.what()}};

    response.set_content(response_json.dump(), "application/json");
  }
}

void handle_find_user(
    const httplib::Request& request, 
    httplib::Response& response, 
    AppContext& context) {
  std::string token = extract_token(request);

  std::optional<std::string> user_id = extract_user_id_from_token(token, context.jwt_secret);

  if (!user_id.has_value()) {
    response.status = 401;
    response.set_content(R"({"error":"Invalid token"})", "application/json");
    return;
  }

  std::string name = request.get_param_value("name");

  if (name.empty()) {
    response.status = 400;
    response.set_content(R"({"error":"Missing 'name' parameter"})", "application/json");
    return;
  }

  std::optional<std::string> found = context.users.find_user_by_name(name);

  if (!found.has_value()) {
    response.status = 404;
    response.set_content(R"({"error":"User not found"})", "application/json");
    return;
  }

  response.status = 200;

  JSON response_json = {{"user_id", *found}};
  
  response.set_content(response_json.dump(), "application/json");
}
