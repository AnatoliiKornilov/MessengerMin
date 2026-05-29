#include "auth_middleware.hpp"

#include <jwt-cpp/jwt.h>

std::optional<std::string> extract_user_id_from_token(
    const std::string& token, 
    const std::string& secret) {
  try {
    auto decoded = jwt::decode(token);

    auto verifier = jwt::verify()
      .allow_algorithm(jwt::algorithm::hs256{secret})
      .with_issuer("messenger");

    verifier.verify(decoded);

    return decoded.get_subject();
  } catch (...) {
    return std::nullopt;
  }
}