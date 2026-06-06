#include "../db/chat_repository.hpp"
#include "../db/user_repository.hpp"
#include "../src/app_context.hpp"
#include "../src/config.hpp"
#include "../handlers/chat_handlers.hpp"
#include "../src/middleware/auth_middleware.hpp"

#include <gtest/gtest.h>
#include <httplib.h>
#include <jwt-cpp/jwt.h>
#include <nlohmann/json.hpp>

#include <memory>
#include <string>

class ChatHandlersTest : public ::testing::Test {
 protected:
  std::unique_ptr<AppContext> ctx;
  std::string secret = "test-secret-for-chats";
  std::string alice_id, bob_id, charlie_id;

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

    alice_id = ctx->users.create_user("alice", "hash_alice");
    bob_id = ctx->users.create_user("bob", "hash_bob");
    charlie_id = ctx->users.create_user("charlie", "hash_charlie");
  }

  std::string make_token(const std::string& user_id) {
    auto now = std::chrono::system_clock::now();
    auto exp = now + std::chrono::minutes(15);
    return jwt::create()
        .set_issuer("messenger")
        .set_subject(user_id)
        .set_issued_at(now)
        .set_expires_at(exp)
        .sign(jwt::algorithm::hs256{secret});
  }

  void call_create_personal_chat(const std::string& token,
                                 const std::string& body,
                                 httplib::Response& res) {
    httplib::Request req;
    req.body = body;
    req.set_header("Content-Type", "application/json");
    if (!token.empty()) {
      req.set_header("Authorization", "Bearer " + token);
    }
    handle_create_personal_chat(req, res, *ctx);
  }

  void call_create_group(const std::string& token, const std::string& body,
                         httplib::Response& res) {
    httplib::Request req;
    req.body = body;
    req.set_header("Content-Type", "application/json");
    if (!token.empty()) {
      req.set_header("Authorization", "Bearer " + token);
    }
    handle_create_group(req, res, *ctx);
  }

  void call_get_chats(const std::string& token, httplib::Response& res) {
    httplib::Request req;
    if (!token.empty()) {
      req.set_header("Authorization", "Bearer " + token);
    }
    handle_get_chats(req, res, *ctx);
  }

  void call_add_member(const std::string& token, const std::string& chat_id,
                       const std::string& body, httplib::Response& res) {
    httplib::Request req;
    req.body = body;
    req.set_header("Content-Type", "application/json");
    if (!token.empty()) {
      req.set_header("Authorization", "Bearer " + token);
    }
    handle_add_member(req, res, *ctx, chat_id);
  }

  void call_remove_member(const std::string& token, const std::string& chat_id,
                          const std::string& member_id,
                          httplib::Response& res) {
    httplib::Request req;
    if (!token.empty()) {
      req.set_header("Authorization", "Bearer " + token);
    }
    handle_remove_member(req, res, *ctx, chat_id, member_id);
  }
};

TEST_F(ChatHandlersTest, CreatePersonalChat_Success) {
  std::string token = make_token(alice_id);
  httplib::Response res;
  call_create_personal_chat(token, R"({"user_name":"bob"})", res);
  ASSERT_EQ(res.status, 201);
  auto json = nlohmann::json::parse(res.body);
  EXPECT_TRUE(json.contains("chat_id"));
  EXPECT_FALSE(json["chat_id"].get<std::string>().empty());
}

TEST_F(ChatHandlersTest, CreatePersonalChat_UserNotFound) {
  std::string token = make_token(alice_id);
  httplib::Response res;
  call_create_personal_chat(token, R"({"user_name":"nonexistent"})", res);
  EXPECT_EQ(res.status, 404);
}

TEST_F(ChatHandlersTest, CreatePersonalChat_NoToken) {
  httplib::Response res;
  call_create_personal_chat("", R"({"user_name":"bob"})", res);
  EXPECT_EQ(res.status, 401);
}

TEST_F(ChatHandlersTest, CreateGroup_Success) {
  std::string token = make_token(alice_id);
  httplib::Response res;
  call_create_group(token, R"({"name":"Test Group"})", res);
  ASSERT_EQ(res.status, 201);
  auto json = nlohmann::json::parse(res.body);
  EXPECT_TRUE(json.contains("chat_id"));
}

TEST_F(ChatHandlersTest, GetChats_ReturnsChats) {
  std::string token = make_token(alice_id);
  httplib::Response res;
  call_create_personal_chat(token, R"({"user_name":"bob"})", res);
  ASSERT_EQ(res.status, 201);
  call_create_group(token, R"({"name":"Group"})", res);
  ASSERT_EQ(res.status, 201);

  httplib::Response chat_res;
  call_get_chats(token, chat_res);
  ASSERT_EQ(chat_res.status, 200);
  auto chats = nlohmann::json::parse(chat_res.body);
  ASSERT_TRUE(chats.is_array());
  EXPECT_EQ(chats.size(), 2);
}

TEST_F(ChatHandlersTest, AddMember_Success) {
  std::string token = make_token(alice_id);
  httplib::Response res;
  call_create_group(token, R"({"name":"Group"})", res);
  std::string group_id = nlohmann::json::parse(res.body)["chat_id"];
  ASSERT_FALSE(group_id.empty());

  httplib::Response add_res;
  call_add_member(token, group_id, R"({"user_name":"bob"})", add_res);
  EXPECT_EQ(add_res.status, 200);
}

TEST_F(ChatHandlersTest, AddMember_ToNonGroup) {
  std::string token = make_token(alice_id);
  httplib::Response res;
  call_create_personal_chat(token, R"({"user_name":"bob"})", res);
  std::string chat_id = nlohmann::json::parse(res.body)["chat_id"];

  httplib::Response add_res;
  call_add_member(token, chat_id, R"({"user_name":"charlie"})", add_res);
  EXPECT_EQ(add_res.status, 400);
}

TEST_F(ChatHandlersTest, RemoveMember_Success) {
  std::string token = make_token(alice_id);
  httplib::Response res;
  call_create_group(token, R"({"name":"Group"})", res);
  std::string group_id = nlohmann::json::parse(res.body)["chat_id"];
  call_add_member(token, group_id, R"({"user_name":"bob"})", res);

  httplib::Response del_res;
  call_remove_member(token, group_id, bob_id, del_res);
  EXPECT_EQ(del_res.status, 200);
}
