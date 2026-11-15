#pragma once

#include <tacit/field/cubic_field.hpp>
#include <tacit/field/prime_field.hpp>
#include <tacit/field/quadratic_field.hpp>

#include <array>

// The fields of BN254 (alt_bn128): the two prime fields, with the moduli from EIP-196 and
// EIP-197, and the extension tower over the base field.

namespace tacit::bn254 {

// Base field of the curve coordinates.
struct FpParams {
  static constexpr std::array<u64, 4> modulus{0x3c208c16d87cfd47, 0x97816a916871ca8d,
                                              0xb85045b68181585d, 0x30644e72e131a029};
};

// Scalar field, which is also the field circuits are written over.
struct FrParams {
  static constexpr std::array<u64, 4> modulus{0x43e1f593f0000001, 0x2833e84879b97091,
                                              0xb85045b68181585d, 0x30644e72e131a029};
};

using Fp = PrimeField<FpParams>;
using Fr = PrimeField<FrParams>;

// Coordinate field of G2 and the first level of the tower up to Fp12.
using Fp2 = QuadraticField<Fp>;

// Fp6 = Fp2[v] / (v^3 - xi) with xi = 9 + u.
struct Fp6Params {
  using Base = Fp2;

  static constexpr Fp2 nonresidue{Fp::from_u64(9), Fp::one()};

  // (9 + u)(a + b u) = (9a - b) + (a + 9b) u, using additions only.
  [[nodiscard]] static constexpr auto mul_by_nonresidue(const Fp2 &x) noexcept -> Fp2 {
    return {times9(x.c0) - x.c1, x.c0 + times9(x.c1)};
  }

private:
  [[nodiscard]] static constexpr auto times9(const Fp &a) noexcept -> Fp {
    const Fp a2 = a + a;
    const Fp a4 = a2 + a2;
    const Fp a8 = a4 + a4;
    return a8 + a;
  }
};

// Second level of the tower up to Fp12.
using Fp6 = CubicField<Fp6Params>;

} // namespace tacit::bn254
