#include <gtest/gtest.h>

#include <cstdlib>
#include <memory>
#include <optional>
#include <pqxx/pqxx>

#include "../db/database.hpp"
#include "../db/user_repository.hpp"

class UserRepoTest : public ::testing::Test {
 protected:
  std::unique_ptr<DataBase> db;
  std::unique_ptr<UserRepository> repo;

  void SetUp() override {
    const char* conn_str = std::getenv("DB_CONN");
    ASSERT_NE(conn_str, nullptr) << "DB_CONN environment variable not set";
    db = std::make_unique<DataBase>(conn_str);
    repo = std::make_unique<UserRepository>(*db);

    auto conn_guard = db->connection();
    pqxx::work txn{*conn_guard.connection};

    txn.exec("DELETE FROM messages");
    txn.exec("DELETE FROM chat_members");
    txn.exec("DELETE FROM chats");
    txn.exec("DELETE FROM users");

    txn.commit();
  }
};

TEST_F(UserRepoTest, CreateUser_ReturnsValidUuid) {
  std::string id = repo->create_user("alice", "hash1");
  EXPECT_EQ(id.length(), 36);
  EXPECT_NE(id.find('-'), std::string::npos);
}

TEST_F(UserRepoTest, FindUserByName_Existing_ReturnsId) {
  std::string id = repo->create_user("bob", "hash2");
  std::optional<std::string> found = repo->find_user_by_name("bob");
  ASSERT_TRUE(found.has_value());
  EXPECT_EQ(found.value(), id);
}

TEST_F(UserRepoTest, FindUserByName_NonExistent_ReturnsNullopt) {
  std::optional<std::string> result = repo->find_user_by_name("ghost");
  EXPECT_FALSE(result.has_value());
}

TEST_F(UserRepoTest, CreateUser_DuplicateName_ThrowsSqlError) {
  repo->create_user("charlie", "hash3");
  EXPECT_THROW({ repo->create_user("charlie", "another_hash"); },
               pqxx::unique_violation);
}

TEST_F(UserRepoTest, GetPasswordHash_ExistingUser_ReturnsHash) {
  std::string password = "secret123";
  std::string user_id = repo->create_user("charlie", password);

  auto retrieved_password = repo->get_password_hash("charlie");
  ASSERT_TRUE(retrieved_password.has_value());
  EXPECT_EQ(retrieved_password.value(), password);
}

TEST_F(UserRepoTest, GetPasswordHash_NonExistingUser_ReturnsNullopt) {
  auto result = repo->get_password_hash("nonexistent");
  EXPECT_FALSE(result.has_value());
}
