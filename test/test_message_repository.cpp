#include <gtest/gtest.h>

#include <cstdlib>
#include <memory>
#include <pqxx/pqxx>

#include "../db/chat_repository.hpp"
#include "../db/database.hpp"
#include "../db/message_repository.hpp"

class MessageRepoTest : public ::testing::Test {
 protected:
  std::unique_ptr<DataBase> db;
  std::unique_ptr<ChatRepository> chat_repo;
  std::unique_ptr<MessageRepository> msg_repo;
  std::string alice_id, bob_id, charlie_id;
  std::string personal_chat_id, group_chat_id;

  void SetUp() override {
    const char* conn_str = std::getenv("DB_CONN");
    ASSERT_NE(conn_str, nullptr) << "DB_CONN not set";
    db = std::make_unique<DataBase>(conn_str);
    chat_repo = std::make_unique<ChatRepository>(*db);
    msg_repo = std::make_unique<MessageRepository>(*db);

    pqxx::work txn{db->connection()};
    txn.exec("DELETE FROM messages");
    txn.exec("DELETE FROM chat_members");
    txn.exec("DELETE FROM chats");
    txn.exec("DELETE FROM users");

    alice_id = txn.exec_params1(
                      "INSERT INTO users (user_name, password_hash) VALUES "
                      "($1,$2) RETURNING user_id",
                      "alice", "h")[0]
                   .as<std::string>();
    bob_id = txn.exec_params1(
                    "INSERT INTO users (user_name, password_hash) VALUES "
                    "($1,$2) RETURNING user_id",
                    "bob", "h")[0]
                 .as<std::string>();
    charlie_id = txn.exec_params1(
                        "INSERT INTO users (user_name, password_hash) VALUES "
                        "($1,$2) RETURNING user_id",
                        "charlie", "h")[0]
                     .as<std::string>();
    txn.commit();

    personal_chat_id = chat_repo->create_personal_chat(alice_id, bob_id);
    group_chat_id = chat_repo->create_group(alice_id, "TestGroup");
  }
};

TEST_F(MessageRepoTest, SendMessage_Success) {
  auto [msg_id, time] =
      msg_repo->send_message(personal_chat_id, alice_id, "Hello Bob!");
  EXPECT_FALSE(msg_id.empty());
  EXPECT_FALSE(time.empty());
}

TEST_F(MessageRepoTest, SendMessage_NotMember_Throws) {
  EXPECT_THROW(msg_repo->send_message(personal_chat_id, charlie_id, "Hi!"),
               std::runtime_error);
}

TEST_F(MessageRepoTest, GetMessages_EmptyList) {
  auto msgs = msg_repo->get_chat_messages(personal_chat_id);
  EXPECT_TRUE(msgs.empty());
}

TEST_F(MessageRepoTest, GetMessages_ReturnsCorrectOrder) {
  msg_repo->send_message(personal_chat_id, alice_id, "First");
  msg_repo->send_message(personal_chat_id, bob_id, "Second");
  msg_repo->send_message(personal_chat_id, alice_id, "Third");

  auto msgs = msg_repo->get_chat_messages(personal_chat_id);
  ASSERT_EQ(msgs.size(), 3);
  EXPECT_EQ(msgs[0].text, "First");
  EXPECT_EQ(msgs[0].sender_name, "alice");
  EXPECT_EQ(msgs[1].text, "Second");
  EXPECT_EQ(msgs[1].sender_name, "bob");
  EXPECT_EQ(msgs[2].text, "Third");
  EXPECT_EQ(msgs[2].sender_name, "alice");
}

TEST_F(MessageRepoTest, GetMessages_Pagination) {
  for (int i = 0; i < 10; ++i) {
    msg_repo->send_message(personal_chat_id, alice_id,
                           "Msg " + std::to_string(i));
  }
  auto page = msg_repo->get_chat_messages(personal_chat_id, 3, 3);
  ASSERT_EQ(page.size(), 3);
  EXPECT_EQ(page[0].text, "Msg 3");
  EXPECT_EQ(page[1].text, "Msg 4");
  EXPECT_EQ(page[2].text, "Msg 5");
}

TEST_F(MessageRepoTest, EditMessage_Success) {
  auto [msg_id, time] =
      msg_repo->send_message(personal_chat_id, alice_id, "Original");
  ASSERT_NO_THROW(msg_repo->edit_message(msg_id, alice_id, "Modified"));

  auto msgs = msg_repo->get_chat_messages(personal_chat_id, 1, 0);
  ASSERT_EQ(msgs.size(), 1);
  EXPECT_EQ(msgs[0].text, "Modified");
}

TEST_F(MessageRepoTest, EditMessage_NotAuthor_Throws) {
  auto [msg_id, time] =
      msg_repo->send_message(personal_chat_id, bob_id, "Bob's message");
  EXPECT_THROW(msg_repo->edit_message(msg_id, alice_id, "Hacked"),
               std::runtime_error);
}

TEST_F(MessageRepoTest, EditMessage_NotFound_Throws) {
  EXPECT_THROW(msg_repo->edit_message("nonexistent-id", alice_id, "Text"),
               std::runtime_error);
}

TEST_F(MessageRepoTest, DeleteMessage_Success) {
  auto [msg_id, time] =
      msg_repo->send_message(personal_chat_id, alice_id, "To delete");
  ASSERT_NO_THROW(msg_repo->delete_message(msg_id, alice_id));

  auto msgs = msg_repo->get_chat_messages(personal_chat_id);
  EXPECT_TRUE(msgs.empty());
}

TEST_F(MessageRepoTest, DeleteMessage_NotAuthor_Throws) {
  auto [msg_id, time] =
      msg_repo->send_message(personal_chat_id, bob_id, "Bob's");
  EXPECT_THROW(msg_repo->delete_message(msg_id, alice_id), std::runtime_error);
}

TEST_F(MessageRepoTest, DeleteMessage_NotFound_Throws) {
  EXPECT_THROW(msg_repo->delete_message("nonexistent-id", alice_id),
               std::runtime_error);
}
