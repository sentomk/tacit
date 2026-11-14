#include <tacit/bn254/fields.hpp>

#include <gtest/gtest.h>
#include <nlohmann/json.hpp>

#include <array>
#include <cstddef>
#include <fstream>
#include <stdexcept>
#include <string>
#include <string_view>

// Checks the BN254 prime fields against the vectors in tests/vectors, which are produced by
// the Sage script and the Python reference models.

namespace {

using tacit::u64;
using Limbs = std::array<u64, 4>;

auto load_vectors(const std::string &file) -> nlohmann::json {
  const std::string path = std::string{TACIT_VECTORS_DIR} + "/" + file;
  std::ifstream in{path};
  if (!in) {
    throw std::runtime_error{"cannot open " + path};
  }
  return nlohmann::json::parse(in);
}

// Parses "0x..." (lowercase, at most 64 digits) into little-endian limbs.
auto parse_hex(std::string_view text) -> Limbs {
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

auto hex(const nlohmann::json &value) -> Limbs { return parse_hex(value.get<std::string>()); }

struct FpCase {
  using Field = tacit::bn254::Fp;
  static constexpr const char *name = "Fp";
};

struct FrCase {
  using Field = tacit::bn254::Fr;
  static constexpr const char *name = "Fr";
};

template <typename Case> class PrimeFieldTest : public ::testing::Test {};

using Cases = ::testing::Types<FpCase, FrCase>;
TYPED_TEST_SUITE(PrimeFieldTest, Cases, );

TYPED_TEST(PrimeFieldTest, DerivedConstantsMatchVectors) {
  using Field = typename TypeParam::Field;
  const auto field = load_vectors("bn254_constants.json").at("fields").at(TypeParam::name);

  EXPECT_EQ(Field::limb_count, field.at("limbs").template get<std::size_t>());
  EXPECT_EQ(Field::modulus, hex(field.at("modulus")));
  EXPECT_EQ(Field::n0, hex(field.at("n0"))[0]);
  EXPECT_EQ(Field::r, hex(field.at("r_mod_p")));
  EXPECT_EQ(Field::r2, hex(field.at("r2_mod_p")));

  const auto limbs = field.at("modulus_limbs");
  ASSERT_EQ(limbs.size(), Field::limb_count);
  for (std::size_t i = 0; i < Field::limb_count; ++i) {
    EXPECT_EQ(Field::modulus[i], hex(limbs.at(i))[0]) << "limb " << i;
  }
}

TYPED_TEST(PrimeFieldTest, ZeroAndOne) {
  using Field = typename TypeParam::Field;

  EXPECT_TRUE(Field::zero().is_zero());
  EXPECT_FALSE(Field::one().is_zero());
  EXPECT_EQ(Field{}, Field::zero());
  EXPECT_EQ(Field::zero().to_limbs(), Limbs{});
  EXPECT_EQ(Field::one().to_limbs(), (Limbs{1, 0, 0, 0}));
  EXPECT_EQ(Field::one().montgomery_limbs(), Field::r);
  EXPECT_EQ(Field::from_u64(1), Field::one());
  EXPECT_EQ(Field::from_u64(7).to_limbs(), (Limbs{7, 0, 0, 0}));
}

TYPED_TEST(PrimeFieldTest, ArithmeticMatchesVectors) {
  using Field = typename TypeParam::Field;
  const auto ops = load_vectors("bn254_field_ops.json").at("fields").at(TypeParam::name).at("ops");
  ASSERT_FALSE(ops.empty());

  for (const auto &c : ops) {
    SCOPED_TRACE("a = " + c.at("a").template get<std::string>() +
                 ", b = " + c.at("b").template get<std::string>());
    const Limbs a_limbs = hex(c.at("a"));
    const Field a = Field::from_limbs(a_limbs);
    const Field b = Field::from_limbs(hex(c.at("b")));

    EXPECT_EQ(a.to_limbs(), a_limbs);
    EXPECT_EQ((a + b).to_limbs(), hex(c.at("add")));
    EXPECT_EQ((a - b).to_limbs(), hex(c.at("sub")));
    EXPECT_EQ((a * b).to_limbs(), hex(c.at("mul")));
    EXPECT_EQ((-a).to_limbs(), hex(c.at("neg_a")));
    EXPECT_EQ(a.square().to_limbs(), hex(c.at("square_a")));

    Field acc = a;
    acc += b;
    EXPECT_EQ(acc, a + b);
    acc -= b;
    EXPECT_EQ(acc, a);
    acc *= b;
    EXPECT_EQ(acc, a * b);
  }
}

TYPED_TEST(PrimeFieldTest, FromLimbsReducesWideInputs) {
  using Field = typename TypeParam::Field;
  const auto cases =
      load_vectors("bn254_field_ops.json").at("fields").at(TypeParam::name).at("reduce");
  ASSERT_FALSE(cases.empty());

  for (const auto &c : cases) {
    SCOPED_TRACE("value = " + c.at("value").template get<std::string>());
    EXPECT_EQ(Field::from_limbs(hex(c.at("value"))).to_limbs(), hex(c.at("reduced")));
  }
}

// The whole interface is usable in constant expressions.
using tacit::bn254::Fr;
static_assert(Fr{}.is_zero());
static_assert(Fr::one() * Fr::one() == Fr::one());
static_assert(Fr::from_u64(6) * Fr::from_u64(7) == Fr::from_u64(42));
static_assert(Fr::from_u64(5) - Fr::from_u64(7) == -Fr::from_u64(2));
static_assert((-Fr::one()).square() == Fr::one());
static_assert(Fr::from_limbs(Fr::modulus).is_zero());

} // namespace
