#pragma once

#include <tacit/field/prime_field.hpp>
#include <tacit/field/quadratic_field.hpp>

#include <array>

// The two prime fields of BN254 (alt_bn128), with the moduli from EIP-196 and EIP-197.

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

} // namespace tacit::bn254
