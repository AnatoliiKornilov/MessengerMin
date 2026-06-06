#include "auth_handlers.hpp"

#include "../src/middleware/auth_middleware.hpp"
#include "../utils/crypto_utils.hpp"

#include <jwt-cpp/jwt.h>
#include <nlohmann/json.hpp>

#include <chrono>
#include <exception>
#include <optional>

using JSON = nlohmann::json;

void handle_register(
    const httplib::Request& request, 
    httplib::Response& response, 
    AppContext& context) {
  try {
    JSON body = nlohmann::json::parse(request.body);

    std::string user_name = body.at("user_name");
    std::string password = body.at("password");

    if (user_name.length() < 3 || password.length() < 8) {
      response.status = 400;
      response.set_content(R"({"error":"Invalid username or password length"})", "application/json");
      return;
    }

    std::string hash = hash_password(password);

    std::string user_id = context.users.create_user(user_name, hash);

    JSON response_json = {{"user_id", user_id}};

    response.status = 201;

    response.set_content(response_json.dump(), "application/json");
  } catch (const pqxx::unique_violation&) {
    response.status = 409;

    response.set_content(R"({"error":"Username already exists"})", "application/json");
  } catch (const std::exception& exception) {
    response.status = 400;

    JSON response_json = {{"error", exception.what()}};
    response.set_content(response_json.dump(), "application/json");
  }
}

void handle_login(
    const httplib::Request& request, 
    httplib::Response& response, 
    AppContext& context) {
  try {
    JSON body = nlohmann::json::parse(request.body);

    std::string user_name = body.at("user_name");
    std::string password = body.at("password");

    if (user_name.length() < 3 || password.length() < 8) {
      response.status = 400;
      response.set_content(R"({"error":"Invalid username or password length"})", "application/json");
      return;
    }

    std::optional<std::string> hash = context.users.get_password_hash(user_name);

    if (!hash.has_value() || !verify_password(*hash, password)) {
      response.status = 401;
      response.set_content(R"({"error":"Invalid password"})", "application/json");
      return;
    }

    std::string user_id = *context.users.find_user_by_name(user_name);

    auto now = std::chrono::system_clock::now();
    auto access_exp = now + std::chrono::minutes(15);
    auto refresh_exp = now + std::chrono::hours(168);

    auto access_token = jwt::create()
      .set_issuer("messenger")
      .set_subject(user_id)
      .set_issued_at(now)
      .set_expires_at(access_exp)
      .sign(jwt::algorithm::hs256{context.jwt_secret});

    auto refresh_token = jwt::create()
      .set_issuer("messenger")
      .set_subject(user_id)
      .set_issued_at(now)
      .set_expires_at(refresh_exp)
      .sign(jwt::algorithm::hs256{context.jwt_secret});

    JSON response_json = {{"access_token", access_token}};

    response.status = 200;

    response.set_content(response_json.dump(), "application/json");

    std::string cookie = 
        "refresh_token=" + 
        refresh_token +
        "; Path=/api/auth; HttpOnly; SameSite=Strict; Max-Age=" +
        std::to_string(60 * 60 * 24 * 7);

    response.set_header("Set-Cookie", cookie);
  } catch (const std::exception& exception) {
    response.status = 400;

    JSON response_json = {{"error", exception.what()}};
    response.set_content(response_json.dump(), "application/json");
  }
}

void handle_refresh(
    const httplib::Request& request, 
    httplib::Response& response, 
    AppContext& context) {
  try {
    std::optional<std::string> refresh_token = extract_refresh_token_from_cookie(request);

    if (!refresh_token.has_value()) {
      response.status = 401;
      response.set_content(R"({"error":"Refresh token missing"})", "application/json");
      return;
    }

    std::optional<std::string> user_id = extract_user_id_from_token(
        *refresh_token, 
        context.jwt_secret);
    
    if (!user_id.has_value()) {
      response.status = 401;
      response.set_content(R"({"error":"Invalid refresh token"})", "application/json");
      return;
    }

    auto now = std::chrono::system_clock::now();
    auto access_exp = now + std::chrono::minutes(15);
    auto refresh_exp = now + std::chrono::hours(168);

    auto new_access = jwt::create()
      .set_issuer("messenger")
      .set_subject(*user_id)
      .set_issued_at(now)
      .set_expires_at(access_exp)
      .sign(jwt::algorithm::hs256{context.jwt_secret});

    auto new_refresh = jwt::create()
      .set_issuer("messenger")
      .set_subject(*user_id)
      .set_issued_at(now)
      .set_expires_at(refresh_exp)
      .sign(jwt::algorithm::hs256{context.jwt_secret});
    
    std::string cookie = 
        "refresh_token=" + 
        new_refresh +
        "; Path=/api/auth; HttpOnly; SameSite=Strict; Max-Age=" +
        std::to_string(60 * 60 * 24 * 7);

    response.set_header("Set-Cookie", cookie);

    JSON response_json = {{"access_token", new_access}};

    response.status = 200;
    response.set_content(response_json.dump(), "application/json");
  } catch (const std::exception& exception) {
    response.status = 400;

    JSON response_json = {{"error", exception.what()}};
    response.set_content(response_json.dump(), "application/json");
  }
}
