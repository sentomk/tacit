#include "test_vectors.hpp"

#include <tacit/bn254/g1.hpp>

#include <gtest/gtest.h>
#include <nlohmann/json.hpp>

#include <array>
#include <string>

// Checks the BN254 group G1 against tests/vectors/bn254_g1_ops.json, produced by the Python
// reference model, and tests/vectors/bn254_g1_eip196.json, taken from go-ethereum's
// precompile tests.

namespace {

using tacit::bn254::Fp;
using tacit::bn254::Fr;
using tacit::bn254::G1;
using tacit::bn254::G1Affine;
using tacit::test::hex;
using tacit::test::load_vectors;

// JSON null is the point at infinity; otherwise [x, y].
auto affine(const nlohmann::json &value) -> G1Affine {
  if (value.is_null()) {
    return {};
  }
  return {Fp::from_limbs(hex(value.at(0))), Fp::from_limbs(hex(value.at(1)))};
}

auto jacobian(const nlohmann::json &value) -> G1 { return G1::from_affine(affine(value)); }

// The same point with z = lambda: (lambda^2 x, lambda^3 y, lambda).
auto rescaled(const G1 &p, const Fp &lambda) -> G1 {
  const Fp l2 = lambda.square();
  return {p.x * l2, p.y * l2 * lambda, p.z * lambda};
}

TEST(Bn254G1, GeneratorHasOrderR) {
  const G1 g = G1::generator();
  EXPECT_TRUE(g.is_on_curve());
  EXPECT_FALSE(g.is_identity());
  EXPECT_TRUE(g.mul(Fr::modulus).is_identity());

  auto r_minus_one = Fr::modulus;
  r_minus_one[0] -= 1;
  EXPECT_EQ(g.mul(r_minus_one), -g);
  EXPECT_EQ(g * -Fr::one(), -g);
  EXPECT_EQ(g * Fr::zero(), G1::identity());
}

TEST(Bn254G1, IdentityBehaves) {
  const G1 o = G1::identity();
  const G1 g = G1::generator();
  EXPECT_TRUE(o.is_identity());
  EXPECT_TRUE(o.is_on_curve());
  EXPECT_TRUE(o.to_affine().is_identity());
  EXPECT_EQ(o + g, g);
  EXPECT_EQ(g + o, g);
  EXPECT_EQ(o + G1Affine::generator(), g);
  EXPECT_EQ(g + G1Affine::identity(), g);
  EXPECT_EQ(o.doubled(), o);
  EXPECT_EQ(-o, o);
  EXPECT_EQ(g - g, o);
  EXPECT_EQ(G1::from_affine(G1Affine::identity()), o);
}

TEST(Bn254G1, EqualityIgnoresTheJacobianScale) {
  const G1 p = G1::generator().mul(std::array<tacit::u64, 1>{12345});
  const G1 q = rescaled(p, Fp::from_u64(7));
  EXPECT_NE(p.z, q.z);
  EXPECT_EQ(p, q);
  EXPECT_TRUE(q.is_on_curve());
  EXPECT_EQ(p.to_affine(), q.to_affine());
  EXPECT_NE(p, -q);
  EXPECT_NE(p, G1::identity());
}

TEST(Bn254G1, AddMatchesVectors) {
  const auto cases = load_vectors("bn254_g1_ops.json").at("add");
  ASSERT_FALSE(cases.empty());

  for (const auto &c : cases) {
    SCOPED_TRACE("a = " + c.at("a").dump() + ", b = " + c.at("b").dump());
    const G1Affine b_affine = affine(c.at("b"));
    const G1 a = jacobian(c.at("a"));
    const G1 b = G1::from_affine(b_affine);
    const G1Affine expected = affine(c.at("sum"));

    EXPECT_EQ((a + b).to_affine(), expected);
    EXPECT_EQ((a + b_affine).to_affine(), expected);
    EXPECT_EQ((rescaled(a, Fp::from_u64(3)) + rescaled(b, Fp::from_u64(5))).to_affine(), expected);
    EXPECT_EQ((rescaled(a, Fp::from_u64(11)) + b_affine).to_affine(), expected);
    EXPECT_EQ(b + a, a + b);
    EXPECT_EQ((a + b) - b, a);
    EXPECT_TRUE((a + b).is_on_curve());
  }
}

TEST(Bn254G1, DoubleMatchesVectors) {
  const auto cases = load_vectors("bn254_g1_ops.json").at("double");
  ASSERT_FALSE(cases.empty());

  for (const auto &c : cases) {
    SCOPED_TRACE("a = " + c.at("a").dump());
    const G1 a = jacobian(c.at("a"));
    EXPECT_EQ(a.doubled().to_affine(), affine(c.at("doubled")));
    EXPECT_EQ(rescaled(a, Fp::from_u64(9)).doubled(), a.doubled());

    G1 self = a;
    self += self;
    EXPECT_EQ(self, a.doubled());
  }
}

TEST(Bn254G1, MulMatchesVectors) {
  const auto cases = load_vectors("bn254_g1_ops.json").at("mul");
  ASSERT_FALSE(cases.empty());

  for (const auto &c : cases) {
    SCOPED_TRACE("base = " + c.at("base").dump() +
                 ", scalar = " + c.at("scalar").get<std::string>());
    const G1 base = jacobian(c.at("base"));
    const auto k = hex(c.at("scalar"));
    const G1Affine expected = affine(c.at("product"));

    EXPECT_EQ(base.mul(k).to_affine(), expected);
    // Every point has order r, so reducing the scalar modulo r gives the same product.
    EXPECT_EQ((base * Fr::from_limbs(k)).to_affine(), expected);
  }
}

TEST(Bn254G1, OffCurvePointsAreRejected) {
  const auto cases = load_vectors("bn254_g1_ops.json").at("off_curve");
  ASSERT_FALSE(cases.empty());

  for (const auto &c : cases) {
    SCOPED_TRACE("point = " + c.dump());
    const G1Affine p = affine(c);
    EXPECT_FALSE(p.is_on_curve());
    EXPECT_FALSE(G1::from_affine(p).is_on_curve());
    EXPECT_FALSE(rescaled(G1::from_affine(p), Fp::from_u64(4)).is_on_curve());
  }
}

TEST(Bn254G1, MatchesEip196Vectors) {
  const auto vectors = load_vectors("bn254_g1_eip196.json");
  ASSERT_FALSE(vectors.at("add").empty());
  ASSERT_FALSE(vectors.at("mul").empty());

  for (const auto &c : vectors.at("add")) {
    SCOPED_TRACE(c.at("name").get<std::string>());
    const G1Affine a = affine(c.at("a"));
    const G1Affine b = affine(c.at("b"));
    ASSERT_TRUE(a.is_on_curve());
    ASSERT_TRUE(b.is_on_curve());
    EXPECT_EQ((G1::from_affine(a) + b).to_affine(), affine(c.at("sum")));
  }

  for (const auto &c : vectors.at("mul")) {
    SCOPED_TRACE(c.at("name").get<std::string>());
    const G1Affine base = affine(c.at("base"));
    ASSERT_TRUE(base.is_on_curve());
    EXPECT_EQ(G1::from_affine(base).mul(hex(c.at("scalar"))).to_affine(), affine(c.at("product")));
  }
}

// The interface is usable in constant expressions.

constexpr G1 g = G1::generator();
constexpr G1 g2 = g.doubled();
constexpr G1 g3 = g2 + g;

static_assert(G1::identity().is_identity());
static_assert(g.is_on_curve() && g2.is_on_curve() && g3.is_on_curve());
static_assert(g + g == g2);
static_assert(g2 + G1Affine::generator() == g3);
static_assert(g3 - g == g2);
static_assert(g.mul(std::array<tacit::u64, 1>{3}) == g3);
static_assert(g * Fr::from_u64(3) == g3);
static_assert(g.to_affine() == G1Affine::generator());
static_assert(-G1Affine::generator() == G1Affine{Fp::one(), -Fp::from_u64(2)});
static_assert(!G1Affine{Fp::one(), Fp::from_u64(3)}.is_on_curve());

} // namespace
