#include "../db/chat_repository.hpp"
#include "../db/database.hpp"

#include <gtest/gtest.h>
#include <pqxx/pqxx>

#include <cstdlib>
#include <memory>
#include <string>

class ChatRepoTest : public ::testing::Test {
 protected:
  std::unique_ptr<DataBase> db;
  std::unique_ptr<ChatRepository> chat_repo;
  std::string alice_id, bob_id, charlie_id;

  void SetUp() override {
    const char* conn_str = std::getenv("DB_CONN");
    ASSERT_NE(conn_str, nullptr) << "DB_CONN not set";
    db = std::make_unique<DataBase>(conn_str);
    chat_repo = std::make_unique<ChatRepository>(*db);

    pqxx::work txn{db->connection()};

    txn.exec("DELETE FROM messages");
    txn.exec("DELETE FROM chat_members");
    txn.exec("DELETE FROM chats");
    txn.exec("DELETE FROM users");

    alice_id = txn.exec_params1(
      "INSERT INTO users (user_name, password_hash) VALUES ($1, $2) RETURNING user_id",
      "alice", "hash1")[0].as<std::string>();

    bob_id = txn.exec_params1(
      "INSERT INTO users (user_name, password_hash) VALUES ($1, $2) RETURNING user_id",
      "bob", "hash2")[0].as<std::string>();

    charlie_id = txn.exec_params1(
      "INSERT INTO users (user_name, password_hash) VALUES ($1, $2) RETURNING user_id",
      "charlie", "hash3")[0].as<std::string>();

    txn.commit();
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
  std::string chat_id = chat_repo->create_group(alice_id, "Test Group");

  pqxx::work txn{db->connection()};
  pqxx::row chat = txn.exec_params1(
      "SELECT is_group, chat_name FROM chats WHERE chat_id=$1", chat_id);
  EXPECT_TRUE(chat[0].as<bool>());
  EXPECT_EQ(chat[1].as<std::string>(), "Test Group");

  pqxx::result members = txn.exec_params(
      "SELECT user_id FROM chat_members WHERE chat_id=$1", chat_id);
  ASSERT_EQ(members.size(), 1);
  EXPECT_EQ(members[0][0].as<std::string>(), alice_id);
}

TEST_F(ChatRepoTest, AddMember_Success) {
  std::string chat_id = chat_repo->create_group(alice_id, "Group");
  EXPECT_NO_THROW(chat_repo->add_member(chat_id, bob_id));

  pqxx::work txn{db->connection()};
  pqxx::result res = txn.exec_params(
      "SELECT 1 FROM chat_members WHERE chat_id=$1 AND user_id=$2",
      chat_id, bob_id);
  EXPECT_FALSE(res.empty());
}

TEST_F(ChatRepoTest, AddMember_DuplicateThrows) {
  std::string chat_id = chat_repo->create_group(alice_id, "Group");
  chat_repo->add_member(chat_id, bob_id);
  EXPECT_THROW(chat_repo->add_member(chat_id, bob_id), std::runtime_error);
}

TEST_F(ChatRepoTest, AddMember_PersonalChatThrows) {
  std::string chat_id = chat_repo->create_personal_chat(alice_id, bob_id);
  EXPECT_THROW(chat_repo->add_member(chat_id, charlie_id), std::runtime_error);
}

TEST_F(ChatRepoTest, RemoveMember_Success) {
  std::string chat_id = chat_repo->create_group(alice_id, "Group");
  chat_repo->add_member(chat_id, bob_id);
  EXPECT_NO_THROW(chat_repo->remove_member(chat_id, bob_id));

  pqxx::work txn{db->connection()};
  pqxx::result res = txn.exec_params(
      "SELECT 1 FROM chat_members WHERE chat_id=$1 AND user_id=$2",
      chat_id, bob_id);
  EXPECT_TRUE(res.empty());
}

TEST_F(ChatRepoTest, RemoveMember_NotMemberThrows) {
  std::string chat_id = chat_repo->create_group(alice_id, "Group");
  EXPECT_THROW(chat_repo->remove_member(chat_id, bob_id), std::runtime_error);
}

TEST_F(ChatRepoTest, RemoveMember_PersonalChatThrows) {
  std::string chat_id = chat_repo->create_personal_chat(alice_id, bob_id);
  EXPECT_THROW(chat_repo->remove_member(chat_id, bob_id), std::runtime_error);
}
