#include "../db/chat_repository.hpp"
#include "../db/database.hpp"
#include "../db/message_repository.hpp"

#include <gtest/gtest.h>
#include <pqxx/pqxx>

#include <algorithm>
#include <cstdlib>
#include <memory>
#include <string>

class ChatRepoTest : public ::testing::Test {
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
                      "($1, $2) RETURNING user_id",
                      "alice", "hash1")[0]
                   .as<std::string>();
    bob_id = txn.exec_params1(
                    "INSERT INTO users (user_name, password_hash) VALUES ($1, "
                    "$2) RETURNING user_id",
                    "bob", "hash2")[0]
                 .as<std::string>();
    charlie_id = txn.exec_params1(
                        "INSERT INTO users (user_name, password_hash) VALUES "
                        "($1, $2) RETURNING user_id",
                        "charlie", "hash3")[0]
                     .as<std::string>();
    txn.commit();

    personal_chat_id = chat_repo->create_personal_chat(alice_id, bob_id);
    group_chat_id = chat_repo->create_group(alice_id, "TestGroup");
  }
};

TEST_F(ChatRepoTest, PersonalChat_SameUsersReturnsSameId) {
  std::string chat1 = chat_repo->create_personal_chat(alice_id, bob_id);
  std::string chat2 = chat_repo->create_personal_chat(alice_id, bob_id);
  EXPECT_EQ(chat1, chat2);
}

TEST_F(ChatRepoTest, PersonalChat_OrderInvariant) {
  std::string chat_ab = chat_repo->create_personal_chat(alice_id, bob_id);
  std::string chat_ba = chat_repo->create_personal_chat(bob_id, alice_id);
  EXPECT_EQ(chat_ab, chat_ba);
}

TEST_F(ChatRepoTest, CreateGroup_ValidIdAndData) {
  std::string chat_id = chat_repo->create_group(alice_id, "Second Group");

  pqxx::work txn{db->connection()};
  pqxx::row chat = txn.exec_params1(
      "SELECT is_group, chat_name FROM chats WHERE chat_id=$1", chat_id);
  EXPECT_TRUE(chat[0].as<bool>());
  EXPECT_EQ(chat[1].as<std::string>(), "Second Group");

  pqxx::result members = txn.exec_params(
      "SELECT user_id FROM chat_members WHERE chat_id=$1", chat_id);
  ASSERT_EQ(members.size(), 1);
  EXPECT_EQ(members[0][0].as<std::string>(), alice_id);
}

TEST_F(ChatRepoTest, AddMember_Success) {
  EXPECT_NO_THROW(chat_repo->add_member(group_chat_id, bob_id));

  pqxx::work txn{db->connection()};
  pqxx::result res = txn.exec_params(
      "SELECT 1 FROM chat_members WHERE chat_id=$1 AND user_id=$2",
      group_chat_id, bob_id);
  EXPECT_FALSE(res.empty());
}

TEST_F(ChatRepoTest, AddMember_DuplicateThrows) {
  chat_repo->add_member(group_chat_id, bob_id);
  EXPECT_THROW(chat_repo->add_member(group_chat_id, bob_id),
               std::runtime_error);
}

TEST_F(ChatRepoTest, AddMember_PersonalChatThrows) {
  EXPECT_THROW(chat_repo->add_member(personal_chat_id, charlie_id),
               std::runtime_error);
}

TEST_F(ChatRepoTest, RemoveMember_Success) {
  chat_repo->add_member(group_chat_id, bob_id);
  EXPECT_NO_THROW(chat_repo->remove_member(group_chat_id, bob_id));

  pqxx::work txn{db->connection()};
  pqxx::result res = txn.exec_params(
      "SELECT 1 FROM chat_members WHERE chat_id=$1 AND user_id=$2",
      group_chat_id, bob_id);
  EXPECT_TRUE(res.empty());
}

TEST_F(ChatRepoTest, RemoveMember_NotMemberThrows) {
  EXPECT_THROW(chat_repo->remove_member(group_chat_id, bob_id),
               std::runtime_error);
}

TEST_F(ChatRepoTest, RemoveMember_PersonalChatThrows) {
  EXPECT_THROW(chat_repo->remove_member(personal_chat_id, bob_id),
               std::runtime_error);
}

TEST_F(ChatRepoTest, GetChatsForUser_ReturnsCorrectData) {
  msg_repo->send_message(personal_chat_id, alice_id, "Hello Bob");

  auto alice_chats = chat_repo->get_chats_for_user(alice_id);
  ASSERT_EQ(alice_chats.size(), 2);

  auto personal = std::find_if(alice_chats.begin(), alice_chats.end(),
                               [](const ChatInfo& c) { return !c.is_group; });
  ASSERT_NE(personal, alice_chats.end());
  EXPECT_EQ(personal->name, "bob");
  EXPECT_EQ(personal->last_message, "Hello Bob");

  auto group = std::find_if(alice_chats.begin(), alice_chats.end(),
                            [](const ChatInfo& c) { return c.is_group; });
  ASSERT_NE(group, alice_chats.end());
  EXPECT_EQ(group->name, "TestGroup");
  EXPECT_TRUE(group->last_message.empty());
}
