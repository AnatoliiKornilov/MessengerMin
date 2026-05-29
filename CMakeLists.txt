cmake_minimum_required(VERSION 3.16)
project(MessengerMin CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(PkgConfig REQUIRED)
pkg_check_modules(PQXX REQUIRED IMPORTED_TARGET libpqxx)

find_package(GTest REQUIRED)

add_library(db_lib STATIC
    db/user_repository.cpp
    db/chat_repository.cpp
    db/message_repository.cpp
)

target_include_directories(db_lib PUBLIC db)
target_link_libraries(db_lib PUBLIC PkgConfig::PQXX)

set(TEST_LIBS db_lib GTest::gtest_main PkgConfig::PQXX)

add_executable(test_user_repository
    test/test_user_repository.cpp
)
target_link_libraries(test_user_repository PRIVATE ${TEST_LIBS})

add_executable(test_chat_repository
    test/test_chat_repository.cpp
)
target_link_libraries(test_chat_repository PRIVATE ${TEST_LIBS})

add_executable(test_message_repository
    test/test_message_repository.cpp
)
target_link_libraries(test_message_repository PRIVATE ${TEST_LIBS})
