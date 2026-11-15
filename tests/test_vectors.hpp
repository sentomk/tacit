#pragma once

#include <tacit/limbs.hpp>

#include <gtest/gtest.h>
#include <nlohmann/json.hpp>

#include <array>
#include <cstddef>
#include <fstream>
#include <stdexcept>
#include <string>
#include <string_view>

// Helpers for reading the JSON files in tests/vectors.

namespace tacit::test {

using Limbs = std::array<u64, 4>;

inline auto load_vectors(const std::string &file) -> nlohmann::json {
  const std::string path = std::string{TACIT_VECTORS_DIR} + "/" + file;
  std::ifstream in{path};
  if (!in) {
    throw std::runtime_error{"cannot open " + path};
  }
  return nlohmann::json::parse(in);
}

// Parses "0x..." (lowercase, at most 64 digits) into little-endian limbs.
inline auto parse_hex(std::string_view text) -> Limbs {
  if (text.substr(0, 2) != "0x" || text.size() < 3 || text.size() > 66) {
    throw std::invalid_argument{"bad hex literal: " + std::string{text}};
  }
  text.remove_prefix(2);

  Limbs out{};
  std::size_t bit = 0;
  for (auto it = text.rbegin(); it != text.rend(); ++it, bit += 4) {
    const char c = *it;
    u64 digit = 0;
    if (c >= '0' && c <= '9') {
      digit = static_cast<u64>(c - '0');
    } else if (c >= 'a' && c <= 'f') {
      digit = static_cast<u64>(c - 'a' + 10);
    } else {
      throw std::invalid_argument{"bad hex digit in: " + std::string{text}};
    }
    out.at(bit / 64) |= digit << (bit % 64);
  }
  return out;
}

inline auto hex(const nlohmann::json &value) -> Limbs {
  return parse_hex(value.get<std::string>());
}

// Checks a.inverse() against a vector entry, where JSON null stands for "no inverse".
template <typename Field, typename Parse>
auto expect_inverse(const Field &a, const nlohmann::json &expected, Parse parse) -> void {
  const auto inverse = a.inverse();
  if (expected.is_null()) {
    EXPECT_TRUE(a.is_zero());
    EXPECT_FALSE(inverse.has_value());
    return;
  }
  ASSERT_TRUE(inverse.has_value());
  EXPECT_EQ(*inverse, parse(expected));
  EXPECT_EQ(a * *inverse, Field::one());
}

} // namespace tacit::test
