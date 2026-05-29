#pragma once
#include <sodium.h>

#include <stdexcept>
#include <string>

inline std::string hash_password(const std::string& password) {
  char hashed[crypto_pwhash_STRBYTES];
  if (crypto_pwhash_str(hashed, password.c_str(), password.length(),
                        crypto_pwhash_OPSLIMIT_MODERATE,
                        crypto_pwhash_MEMLIMIT_MODERATE) != 0) {
    throw std::runtime_error("Password hashing failed");
  }
  return std::string(hashed);
}

inline bool verify_password(const std::string& hashed,
                            const std::string& password) {
  return crypto_pwhash_str_verify(hashed.c_str(), password.c_str(),
                                  password.length()) == 0;
}
