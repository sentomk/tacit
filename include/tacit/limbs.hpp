#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

// Multi-precision building blocks on little-endian arrays of 64-bit limbs.

namespace tacit {

using u64 = std::uint64_t;

namespace limbs {

__extension__ typedef unsigned __int128 u128;

// Returns the low word of a + b + carry and stores the high word in carry.
[[nodiscard]] constexpr auto adc(u64 a, u64 b, u64 &carry) noexcept -> u64 {
  const u128 sum = static_cast<u128>(a) + b + carry;
  carry = static_cast<u64>(sum >> 64);
  return static_cast<u64>(sum);
}

// Returns the low word of a - b - borrow and stores the outgoing borrow (0 or 1) in borrow.
[[nodiscard]] constexpr auto sbb(u64 a, u64 b, u64 &borrow) noexcept -> u64 {
  const u128 diff = static_cast<u128>(a) - b - borrow;
  borrow = static_cast<u64>(diff >> 64) & 1U;
  return static_cast<u64>(diff);
}

// Returns the low word of acc + a * b + carry and stores the high word in carry.
// The sum cannot overflow 128 bits: (2^64 - 1)^2 + 2 * (2^64 - 1) = 2^128 - 1.
[[nodiscard]] constexpr auto mac(u64 acc, u64 a, u64 b, u64 &carry) noexcept -> u64 {
  const u128 wide = static_cast<u128>(a) * b + acc + carry;
  carry = static_cast<u64>(wide >> 64);
  return static_cast<u64>(wide);
}

// a += b. Returns the carry out of the top limb.
template <std::size_t N>
constexpr auto add_assign(std::array<u64, N> &a, const std::array<u64, N> &b) noexcept -> u64 {
  u64 carry = 0;
  for (std::size_t i = 0; i < N; ++i) {
    a[i] = adc(a[i], b[i], carry);
  }
  return carry;
}

// a -= b. Returns the borrow out of the top limb.
template <std::size_t N>
constexpr auto sub_assign(std::array<u64, N> &a, const std::array<u64, N> &b) noexcept -> u64 {
  u64 borrow = 0;
  for (std::size_t i = 0; i < N; ++i) {
    a[i] = sbb(a[i], b[i], borrow);
  }
  return borrow;
}

} // namespace limbs

} // namespace tacit
