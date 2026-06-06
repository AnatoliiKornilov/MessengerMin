#include "../db/chat_repository.hpp"
#include "../db/user_repository.hpp"
#include "../handlers/message_handlers.hpp"
#include "../src/app_context.hpp"
#include "../src/config.hpp"
#include "../src/middleware/auth_middleware.hpp"

#include <gtest/gtest.h>
#include <httplib.h>
#include <jwt-cpp/jwt.h>
#include <nlohmann/json.hpp>

#include <memory>
#include <string>

class MessageHandlersTest : public ::testing::Test {
 protected:
  std::unique_ptr<AppContext> ctx;
  std::string secret = "test-secret-msg";
  std::string alice_id, bob_id;
  std::string chat_id;

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

    alice_id = ctx->users.create_user("alice", "hash");
    bob_id = ctx->users.create_user("bob", "hash");
    chat_id = ctx->chats.create_personal_chat(alice_id, bob_id);
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

  void call_send_message(const std::string& token,
                         const std::string& chat_id_param,
                         const std::string& body, httplib::Response& res) {
    httplib::Request req;
    req.body = body;
    req.set_header("Content-Type", "application/json");
    if (!token.empty()) req.set_header("Authorization", "Bearer " + token);
    handle_send_message(req, res, *ctx, chat_id_param);
  }

  void call_get_messages(const std::string& token,
                         const std::string& chat_id_param,
                         httplib::Response& res) {
    httplib::Request req;
    if (!token.empty()) req.set_header("Authorization", "Bearer " + token);
    handle_get_messages(req, res, *ctx, chat_id_param);
  }

  void call_edit_message(const std::string& token, const std::string& msg_id,
                         const std::string& body, httplib::Response& res) {
    httplib::Request req;
    req.body = body;
    req.set_header("Content-Type", "application/json");
    if (!token.empty()) req.set_header("Authorization", "Bearer " + token);
    handle_edit_message(req, res, *ctx, msg_id);
  }

  void call_delete_message(const std::string& token, const std::string& msg_id,
                           httplib::Response& res) {
    httplib::Request req;
    if (!token.empty()) req.set_header("Authorization", "Bearer " + token);
    handle_delete_message(req, res, *ctx, msg_id);
  }
};

TEST_F(MessageHandlersTest, SendMessage_Success) {
  std::string token = make_token(alice_id);
  httplib::Response res;
  call_send_message(token, chat_id, R"({"text":"Hello"})", res);
  ASSERT_EQ(res.status, 201);
  auto json = nlohmann::json::parse(res.body);
  EXPECT_TRUE(json.contains("message_id"));
  EXPECT_TRUE(json.contains("sent_at"));
}

TEST_F(MessageHandlersTest, SendMessage_NoToken) {
  httplib::Response res;
  call_send_message("", chat_id, R"({"text":"Hi"})", res);
  EXPECT_EQ(res.status, 401);
}

TEST_F(MessageHandlersTest, GetMessages_Empty) {
  std::string token = make_token(alice_id);
  httplib::Response res;
  call_get_messages(token, chat_id, res);
  ASSERT_EQ(res.status, 200);
  auto arr = nlohmann::json::parse(res.body);
  EXPECT_TRUE(arr.is_array());
  EXPECT_TRUE(arr.empty());
}

TEST_F(MessageHandlersTest, GetMessages_WithMessages) {
  std::string token = make_token(alice_id);
  httplib::Response res;
  call_send_message(token, chat_id, R"({"text":"First"})", res);
  ASSERT_EQ(res.status, 201);
  call_send_message(token, chat_id, R"({"text":"Second"})", res);

  httplib::Response msg_res;
  call_get_messages(token, chat_id, msg_res);
  ASSERT_EQ(msg_res.status, 200);
  auto arr = nlohmann::json::parse(msg_res.body);
  ASSERT_EQ(arr.size(), 2);
  EXPECT_EQ(arr[0]["text"], "First");
  EXPECT_EQ(arr[1]["text"], "Second");
}

TEST_F(MessageHandlersTest, EditMessage_Success) {
  std::string token_alice = make_token(alice_id);
  httplib::Response res;
  call_send_message(token_alice, chat_id, R"({"text":"Old"})", res);
  std::string msg_id = nlohmann::json::parse(res.body)["message_id"];

  httplib::Response edit_res;
  call_edit_message(token_alice, msg_id, R"({"text":"New"})", edit_res);
  EXPECT_EQ(edit_res.status, 200);

  httplib::Response get_res;
  call_get_messages(token_alice, chat_id, get_res);
  auto arr = nlohmann::json::parse(get_res.body);
  EXPECT_EQ(arr[0]["text"], "New");
}

TEST_F(MessageHandlersTest, EditMessage_NotAuthor) {
  std::string token_alice = make_token(alice_id);
  httplib::Response res;
  call_send_message(token_alice, chat_id, R"({"text":"Alice msg"})", res);
  std::string msg_id = nlohmann::json::parse(res.body)["message_id"];

  std::string token_bob = make_token(bob_id);
  httplib::Response edit_res;
  call_edit_message(token_bob, msg_id, R"({"text":"Hacked"})", edit_res);
  EXPECT_EQ(edit_res.status, 403);
}

TEST_F(MessageHandlersTest, DeleteMessage_Success) {
  std::string token_alice = make_token(alice_id);
  httplib::Response res;
  call_send_message(token_alice, chat_id, R"({"text":"To delete"})", res);
  std::string msg_id = nlohmann::json::parse(res.body)["message_id"];

  httplib::Response del_res;
  call_delete_message(token_alice, msg_id, del_res);
  EXPECT_EQ(del_res.status, 200);

  httplib::Response get_res;
  call_get_messages(token_alice, chat_id, get_res);
  auto arr = nlohmann::json::parse(get_res.body);
  EXPECT_TRUE(arr.empty());
}

TEST_F(MessageHandlersTest, DeleteMessage_NotAuthor) {
  std::string token_alice = make_token(alice_id);
  httplib::Response res;
  call_send_message(token_alice, chat_id, R"({"text":"Alice msg"})", res);
  std::string msg_id = nlohmann::json::parse(res.body)["message_id"];

  std::string token_bob = make_token(bob_id);
  httplib::Response del_res;
  call_delete_message(token_bob, msg_id, del_res);
  EXPECT_EQ(del_res.status, 403);
}
