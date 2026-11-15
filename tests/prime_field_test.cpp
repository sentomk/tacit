#include "test_vectors.hpp"

#include <tacit/bn254/fields.hpp>

#include <gtest/gtest.h>
#include <nlohmann/json.hpp>

#include <array>
#include <cstddef>
#include <string>

// Checks the BN254 prime fields against the vectors in tests/vectors, which are produced by
// the Sage script and the Python reference models.

namespace {

using tacit::test::expect_inverse;
using tacit::test::hex;
using tacit::test::Limbs;
using tacit::test::load_vectors;

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
    expect_inverse(a, c.at("inverse_a"),
                   [](const nlohmann::json &j) { return Field::from_limbs(hex(j)); });

    Field acc = a;
    acc += b;
    EXPECT_EQ(acc, a + b);
    acc -= b;
    EXPECT_EQ(acc, a);
    acc *= b;
    EXPECT_EQ(acc, a * b);
  }
}

TYPED_TEST(PrimeFieldTest, PowMatchesVectors) {
  using Field = typename TypeParam::Field;
  const auto cases =
      load_vectors("bn254_field_ops.json").at("fields").at(TypeParam::name).at("pow");
  ASSERT_FALSE(cases.empty());

  for (const auto &c : cases) {
    SCOPED_TRACE("base = " + c.at("base").template get<std::string>() +
                 ", exponent = " + c.at("exponent").template get<std::string>());
    const Field base = Field::from_limbs(hex(c.at("base")));
    EXPECT_EQ(base.pow(hex(c.at("exponent"))).to_limbs(), hex(c.at("result")));
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
static_assert(Fr::from_u64(3).pow(std::array<tacit::u64, 1>{4}) == Fr::from_u64(81));
static_assert(Fr::from_u64(2).inverse().value() * Fr::from_u64(2) == Fr::one());
static_assert(Fr::one().inverse() == Fr::one());
static_assert(!Fr::zero().inverse().has_value());

} // namespace
