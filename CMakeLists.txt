cmake_minimum_required(VERSION 3.16)
project(MessengerMin CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(PkgConfig REQUIRED)
pkg_check_modules(PQXX REQUIRED IMPORTED_TARGET libpqxx)
pkg_check_modules(SODIUM REQUIRED IMPORTED_TARGET libsodium)

find_package(GTest REQUIRED)

include(FetchContent)

FetchContent_Declare(
    httplib
    GIT_REPOSITORY https://github.com/yhirose/cpp-httplib.git
    GIT_TAG v0.14.3
)
FetchContent_MakeAvailable(httplib)

FetchContent_Declare(
    jwt-cpp
    GIT_REPOSITORY https://github.com/Thalhammer/jwt-cpp.git
    GIT_TAG v0.7.0
)
FetchContent_MakeAvailable(jwt-cpp)

FetchContent_Declare(
    json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.11.2
)
FetchContent_MakeAvailable(json)

add_library(db_lib STATIC
    db/user_repository.cpp
    db/chat_repository.cpp
    db/message_repository.cpp
)
target_include_directories(db_lib PUBLIC db)
target_link_libraries(db_lib PUBLIC PkgConfig::PQXX)

add_executable(test_user_repository test/test_user_repository.cpp)
add_executable(test_chat_repository test/test_chat_repository.cpp)
add_executable(test_message_repository test/test_message_repository.cpp)

set(TEST_LIBS db_lib GTest::gtest_main PkgConfig::PQXX)

target_link_libraries(test_user_repository PRIVATE ${TEST_LIBS})
target_link_libraries(test_chat_repository PRIVATE ${TEST_LIBS})
target_link_libraries(test_message_repository PRIVATE ${TEST_LIBS})

add_executable(messenger_server
    src/main.cpp
    src/app.cpp
    src/handlers/auth_handlers.cpp
    src/handlers/chat_handlers.cpp
    src/handlers/message_handlers.cpp
    src/middleware/auth_middleware.cpp
)
target_include_directories(messenger_server PRIVATE src)
target_link_libraries(messenger_server PRIVATE
    db_lib
    httplib::httplib
    jwt-cpp
    nlohmann_json::nlohmann_json
    PkgConfig::SODIUM
    PkgConfig::PQXX
)
