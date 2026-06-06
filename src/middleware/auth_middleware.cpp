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

std::optional<std::string> extract_refresh_token_from_cookie(const httplib::Request& request) {
  if (!request.has_header("Cookie")) {
    return std::nullopt;
  }

  std::string cookie_header = request.get_header_value("Cookie");

  std::size_t pos = cookie_header.find("refresh_token=");
  
  if (pos == std::string::npos) {
    return std::nullopt;
  }

  pos += 14;
  std::size_t end = cookie_header.find(';', pos);
  return cookie_header.substr(pos, end - pos);
}
