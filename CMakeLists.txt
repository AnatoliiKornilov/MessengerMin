cmake_minimum_required(VERSION 3.16)
project(MessengerMin CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(PkgConfig REQUIRED)
pkg_check_modules(PQXX   REQUIRED IMPORTED_TARGET libpqxx)
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
set(CMAKE_DISABLE_FIND_PACKAGE_jsoncons ON CACHE BOOL "Disable jsoncons search" FORCE)
set(JWT_DISABLE_JSONCONS ON CACHE BOOL "Disable jsoncons" FORCE)
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


set(SERVER_LOGIC_SOURCES
    src/middleware/auth_middleware.cpp
    handlers/auth_handlers.cpp
    handlers/chat_handlers.cpp
)
add_library(server_logic STATIC ${SERVER_LOGIC_SOURCES})
target_include_directories(server_logic
    PUBLIC
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}
)
target_link_libraries(server_logic
    PUBLIC
        db_lib
        PkgConfig::PQXX
        PkgConfig::SODIUM
        httplib::httplib
        jwt-cpp
        nlohmann_json::nlohmann_json
)

set(DB_TEST_LIBS
    db_lib
    GTest::gtest_main
    PkgConfig::PQXX
)

add_executable(test_user_repository test/test_user_repository.cpp)
target_link_libraries(test_user_repository PRIVATE ${DB_TEST_LIBS})

add_executable(test_chat_repository test/test_chat_repository.cpp)
target_link_libraries(test_chat_repository PRIVATE ${DB_TEST_LIBS})

add_executable(test_message_repository test/test_message_repository.cpp)
target_link_libraries(test_message_repository PRIVATE ${DB_TEST_LIBS})

add_executable(test_auth_handlers test/test_auth_handlers.cpp)
target_link_libraries(test_auth_handlers PRIVATE server_logic GTest::gtest_main)

add_executable(test_chat_handlers test/test_chat_handlers.cpp)
target_link_libraries(test_chat_handlers PRIVATE server_logic GTest::gtest_main)
