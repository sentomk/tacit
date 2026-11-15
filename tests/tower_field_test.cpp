#include "test_vectors.hpp"

#include <tacit/bn254/fields.hpp>

#include <gtest/gtest.h>
#include <nlohmann/json.hpp>

// Checks BN254 Fp2, Fp6 and Fp12 against tests/vectors/bn254_tower_ops.json, which the Python
// reference model produces with schoolbook arithmetic.

namespace {

using tacit::bn254::Fp;
using tacit::bn254::Fp12;
using tacit::bn254::Fp12Params;
using tacit::bn254::Fp2;
using tacit::bn254::Fp6;
using tacit::bn254::Fp6Params;
using tacit::test::expect_inverse;
using tacit::test::hex;
using tacit::test::load_vectors;

auto fp(const nlohmann::json &value) -> Fp { return Fp::from_limbs(hex(value)); }

auto fp2(const nlohmann::json &value) -> Fp2 { return {fp(value.at(0)), fp(value.at(1))}; }

auto fp6(const nlohmann::json &value) -> Fp6 {
  return {fp2(value.at(0)), fp2(value.at(1)), fp2(value.at(2))};
}

TEST(Bn254Fp2, ArithmeticMatchesVectors) {
  const auto ops = load_vectors("bn254_tower_ops.json").at("fp2").at("ops");
  ASSERT_FALSE(ops.empty());

  for (const auto &c : ops) {
    SCOPED_TRACE("a = " + c.at("a").dump() + ", b = " + c.at("b").dump());
    const Fp2 a = fp2(c.at("a"));
    const Fp2 b = fp2(c.at("b"));

    EXPECT_EQ(a + b, fp2(c.at("add")));
    EXPECT_EQ(a - b, fp2(c.at("sub")));
    EXPECT_EQ(a * b, fp2(c.at("mul")));
    EXPECT_EQ(-a, fp2(c.at("neg_a")));
    EXPECT_EQ(a.square(), fp2(c.at("square_a")));
    expect_inverse(a, c.at("inverse_a"), fp2);
    EXPECT_EQ(a.conjugate(), fp2(c.at("conjugate_a")));
    EXPECT_EQ(a.norm(), fp(c.at("norm_a")));
    EXPECT_EQ(a * b.c0, fp2(c.at("a_scaled_by_b0")));

    Fp2 acc = a;
    acc += b;
    EXPECT_EQ(acc, a + b);
    acc -= b;
    EXPECT_EQ(acc, a);
    acc *= b;
    EXPECT_EQ(acc, a * b);
    acc = a;
    acc *= acc;
    EXPECT_EQ(acc, a.square());
  }
}

TEST(Bn254Fp6, NonresidueMatchesVectors) {
  const auto vectors = load_vectors("bn254_tower_ops.json");
  const Fp2 xi = fp2(vectors.at("fp6").at("nonresidue"));
  EXPECT_EQ(Fp6Params::nonresidue, xi);

  for (const auto &c : vectors.at("fp2").at("ops")) {
    SCOPED_TRACE("a = " + c.at("a").dump());
    const Fp2 a = fp2(c.at("a"));
    EXPECT_EQ(Fp6Params::mul_by_nonresidue(a), fp2(c.at("a_times_xi")));
    EXPECT_EQ(Fp6Params::mul_by_nonresidue(a), a * xi);
  }
}

TEST(Bn254Fp6, ArithmeticMatchesVectors) {
  const auto ops = load_vectors("bn254_tower_ops.json").at("fp6").at("ops");
  ASSERT_FALSE(ops.empty());

  for (const auto &c : ops) {
    SCOPED_TRACE("a = " + c.at("a").dump() + ", b = " + c.at("b").dump());
    const Fp6 a = fp6(c.at("a"));
    const Fp6 b = fp6(c.at("b"));

    EXPECT_EQ(a + b, fp6(c.at("add")));
    EXPECT_EQ(a - b, fp6(c.at("sub")));
    EXPECT_EQ(a * b, fp6(c.at("mul")));
    EXPECT_EQ(-a, fp6(c.at("neg_a")));
    EXPECT_EQ(a.square(), fp6(c.at("square_a")));
    expect_inverse(a, c.at("inverse_a"), fp6);
    EXPECT_EQ(a * b.c0, fp6(c.at("a_scaled_by_b0")));
    EXPECT_EQ(a.mul_by_v(), fp6(c.at("a_times_v")));

    Fp6 acc = a;
    acc += b;
    EXPECT_EQ(acc, a + b);
    acc -= b;
    EXPECT_EQ(acc, a);
    acc *= b;
    EXPECT_EQ(acc, a * b);
    acc = a;
    acc *= acc;
    EXPECT_EQ(acc, a.square());
    acc = a;
    acc *= acc.c1;
    EXPECT_EQ(acc, a * a.c1);
  }
}

auto fp12(const nlohmann::json &value) -> Fp12 { return {fp6(value.at(0)), fp6(value.at(1))}; }

TEST(Bn254Fp12, ArithmeticMatchesVectors) {
  const auto ops = load_vectors("bn254_tower_ops.json").at("fp12").at("ops");
  ASSERT_FALSE(ops.empty());

  for (const auto &c : ops) {
    SCOPED_TRACE("a = " + c.at("a").dump() + ", b = " + c.at("b").dump());
    const Fp12 a = fp12(c.at("a"));
    const Fp12 b = fp12(c.at("b"));

    EXPECT_EQ(a + b, fp12(c.at("add")));
    EXPECT_EQ(a - b, fp12(c.at("sub")));
    EXPECT_EQ(a * b, fp12(c.at("mul")));
    EXPECT_EQ(-a, fp12(c.at("neg_a")));
    EXPECT_EQ(a.square(), fp12(c.at("square_a")));
    expect_inverse(a, c.at("inverse_a"), fp12);
    EXPECT_EQ(a.conjugate(), fp12(c.at("conjugate_a")));
    EXPECT_EQ(a * b.c0, fp12(c.at("a_scaled_by_b0")));

    Fp12 acc = a;
    acc += b;
    EXPECT_EQ(acc, a + b);
    acc -= b;
    EXPECT_EQ(acc, a);
    acc *= b;
    EXPECT_EQ(acc, a * b);
    acc = a;
    acc *= acc;
    EXPECT_EQ(acc, a.square());
    acc = a;
    acc *= acc.c1;
    EXPECT_EQ(acc, a * a.c1);
  }
}

// The interface is usable in constant expressions.

constexpr Fp2 u{Fp::zero(), Fp::one()};
constexpr Fp2 x{Fp::from_u64(3), Fp::from_u64(4)};

static_assert(Fp2{}.is_zero());
static_assert(u * u == -Fp2::one());
static_assert(u.square() == -Fp2::one());
static_assert(x * x == x.square());
static_assert(x * x.conjugate() == Fp2{Fp::from_u64(25), Fp::zero()});
static_assert(x.norm() == Fp::from_u64(25));
static_assert(x * Fp2::one() == x);
static_assert((x + u) - u == x);

// Scaling by one of the element's own coefficients.
constexpr auto scaled_by_own_c0 = [] {
  Fp2 y = x;
  y *= y.c0;
  return y;
}();
static_assert(scaled_by_own_c0 == Fp2{Fp::from_u64(9), Fp::from_u64(12)});

constexpr Fp6 v{Fp2::zero(), Fp2::one(), Fp2::zero()};
constexpr Fp6 y{x, u, x + u};

static_assert(Fp6{}.is_zero());
static_assert(v * v * v == Fp6{Fp6Params::nonresidue, Fp2::zero(), Fp2::zero()});
static_assert(y * Fp6::one() == y);
static_assert(y * y == y.square());
static_assert(y.mul_by_v() == y * v);
static_assert((y + v) - v == y);
static_assert(y + (-y) == Fp6::zero());

constexpr Fp12 w{Fp6::zero(), Fp6::one()};
constexpr Fp12 z{y, y + v};

static_assert(Fp12{}.is_zero());
static_assert(Fp12Params::nonresidue == v);
static_assert(w * w == Fp12{v, Fp6::zero()});
static_assert(w.square() == Fp12{v, Fp6::zero()});
static_assert(z * Fp12::one() == z);
static_assert(z * z == z.square());
static_assert((z * z.conjugate()).c1.is_zero());
static_assert((z + w) - w == z);
static_assert(z + (-z) == Fp12::zero());

// Inverses through the whole tower.
static_assert(x * x.inverse().value() == Fp2::one());
static_assert(y * y.inverse().value() == Fp6::one());
static_assert(z * z.inverse().value() == Fp12::one());
static_assert(u.inverse() == -u);
static_assert(!Fp2::zero().inverse().has_value());
static_assert(!Fp6::zero().inverse().has_value());
static_assert(!Fp12::zero().inverse().has_value());

} // namespace
