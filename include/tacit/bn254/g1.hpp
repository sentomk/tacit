#pragma once

#include <tacit/bn254/fields.hpp>
#include <tacit/curve/short_weierstrass.hpp>

// The group G1 of BN254: the curve y^2 = x^3 + 3 over Fp with generator (1, 2), as in
// EIP-196. Every point of the curve over Fp lies in G1, whose order is the modulus of Fr.

namespace tacit::bn254 {

struct G1Params {
  using Field = Fp;
  using Scalar = Fr;

  static constexpr Fp b = Fp::from_u64(3);
  static constexpr Fp generator_x = Fp::from_u64(1);
  static constexpr Fp generator_y = Fp::from_u64(2);
};

using G1Affine = AffinePoint<G1Params>;
using G1 = JacobianPoint<G1Params>;

static_assert(G1Affine::generator().is_on_curve());

} // namespace tacit::bn254
