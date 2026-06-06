#include "../db/user_repository.hpp"
#include "../handlers/auth_handlers.hpp"
#include "../src/app_context.hpp"
#include "../src/config.hpp"

#include <gtest/gtest.h>
#include "httplib.h"
#include <nlohmann/json.hpp>

#include <memory>
#include <string>

class AuthHandlersTest : public ::testing::Test {
 protected:
  std::unique_ptr<AppContext> ctx;
  std::string secret = "test-secret-key-32bytes-long!";

  void SetUp() override {
    const char* db_conn = std::getenv("DB_CONN");
    ASSERT_NE(db_conn, nullptr) << "DB_CONN not set";
    Config cfg;
    cfg.db_connection_string = db_conn;
    cfg.jwt_secret = secret;
    cfg.db_pool_size = 2;

    ctx = std::make_unique<AppContext>(cfg);

    auto conn_guard = ctx->db.connection();
    pqxx::work txn{*conn_guard.connection};
    txn.exec("DELETE FROM messages");
    txn.exec("DELETE FROM chat_members");
    txn.exec("DELETE FROM chats");
    txn.exec("DELETE FROM users");
    txn.commit();
  }

  void call_register(const std::string& body, httplib::Response& res) {
    httplib::Request req;
    req.body = body;
    req.set_header("Content-Type", "application/json");
    handle_register(req, res, *ctx);
  }

  void call_login(const std::string& body, httplib::Response& res) {
    httplib::Request req;
    req.body = body;
    req.set_header("Content-Type", "application/json");
    handle_login(req, res, *ctx);
  }

  void call_refresh(const std::string& cookie_value, httplib::Response& res) {
    httplib::Request req;
    if (!cookie_value.empty()) {
        req.set_header("Cookie", "refresh_token=" + cookie_value);
    }
    handle_refresh(req, res, *ctx);
  }

  std::string extract_refresh_token(const httplib::Response& res) {
    std::string set_cookie = res.get_header_value("Set-Cookie");
    auto pos = set_cookie.find("refresh_token=");
    if (pos == std::string::npos) return "";
    pos += 14;
    auto end = set_cookie.find(';', pos);
    return set_cookie.substr(pos, end - pos);
  }
};

TEST_F(AuthHandlersTest, Register_Success) {
  httplib::Response res;
  call_register(R"({"user_name":"alice","password":"password123"})", res);
  ASSERT_EQ(res.status, 201);
  auto json = nlohmann::json::parse(res.body);
  EXPECT_TRUE(json.contains("user_id"));
  EXPECT_FALSE(json["user_id"].get<std::string>().empty());
}

TEST_F(AuthHandlersTest, Register_Duplicate) {
  httplib::Response res;
  call_register(R"({"user_name":"alice","password":"password123"})", res);
  httplib::Response res2;
  call_register(R"({"user_name":"alice","password":"password456"})", res2);
  EXPECT_EQ(res2.status, 409);
  auto json = nlohmann::json::parse(res2.body);
  EXPECT_EQ(json["error"], "Username already exists");
}

TEST_F(AuthHandlersTest, Register_InvalidInput) {
  httplib::Response res;
  call_register(R"({"user_name":"a","password":"123"})", res);
  EXPECT_EQ(res.status, 400);
}

TEST_F(AuthHandlersTest, Login_Success) {
  httplib::Response res;
  call_register(R"({"user_name":"bob","password":"bobspassword"})", res);

  httplib::Response login_res;
  call_login(R"({"user_name":"bob","password":"bobspassword"})", login_res);
  ASSERT_EQ(login_res.status, 200);
  auto json = nlohmann::json::parse(login_res.body);
  EXPECT_TRUE(json.contains("access_token"));

  auto set_cookie = login_res.get_header_value("Set-Cookie");
  EXPECT_TRUE(set_cookie.find("refresh_token=") != std::string::npos);
  EXPECT_TRUE(set_cookie.find("HttpOnly") != std::string::npos);
}

TEST_F(AuthHandlersTest, Login_WrongPassword) {
  httplib::Response res;
  call_register(R"({"user_name":"bob","password":"right"})", res);
  httplib::Response login_res;
  call_login(R"({"user_name":"bob","password":"wrong_password"})", login_res);
  EXPECT_EQ(login_res.status, 401);
}

TEST_F(AuthHandlersTest, Login_UserNotFound) {
  httplib::Response res;
  call_login(R"({"user_name":"ghost","password":"12345678"})", res);
  EXPECT_EQ(res.status, 401);
}

TEST_F(AuthHandlersTest, Refresh_Success) {
  httplib::Response res;
  call_register(R"({"user_name":"bob","password":"bobspassword"})", res);
  httplib::Response login_res;
  call_login(R"({"user_name":"bob","password":"bobspassword"})", login_res);
  ASSERT_EQ(login_res.status, 200);
  std::string old_refresh = extract_refresh_token(login_res);
  ASSERT_FALSE(old_refresh.empty());

  httplib::Response refresh_res;
  call_refresh(old_refresh, refresh_res);
  ASSERT_EQ(refresh_res.status, 200);

  auto json = nlohmann::json::parse(refresh_res.body);
  EXPECT_TRUE(json.contains("access_token"));

  std::string new_cookie = refresh_res.get_header_value("Set-Cookie");
  EXPECT_TRUE(new_cookie.find("refresh_token=") != std::string::npos);
  std::string new_refresh = extract_refresh_token(refresh_res);
  EXPECT_FALSE(new_refresh.empty());
}

TEST_F(AuthHandlersTest, Refresh_MissingCookie) {
  httplib::Response res;
  call_refresh("", res);
  EXPECT_EQ(res.status, 401);
}

TEST_F(AuthHandlersTest, Refresh_InvalidToken) {
  httplib::Response res;
  call_refresh("not.a.valid.token", res);
  EXPECT_EQ(res.status, 401);
}
