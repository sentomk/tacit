#pragma once

#include <tacit/limbs.hpp>

#include <array>
#include <cstddef>
#include <optional>

// Prime field arithmetic in Montgomery form, parameterized at compile time.
//
// A field is described by a parameter type that provides only the modulus:
//
//     struct MyParams {
//         static constexpr std::array<tacit::u64, 4> modulus{...}; // little-endian limbs
//     };
//     using MyField = tacit::PrimeField<MyParams>;
//
// The Montgomery constants are derived from the modulus during compilation, with
// R = 2^(64 * limb_count).

namespace tacit {

namespace detail {

// -p^-1 mod 2^64 for odd p, given the lowest limb of p.
[[nodiscard]] constexpr auto neg_inv_word(u64 p0) noexcept -> u64 {
  // Newton iteration: each step doubles the number of correct low bits. p0 is its own
  // inverse modulo 8, so five steps give 3 * 2^5 = 96 >= 64 bits.
  u64 inv = p0;
  for (int i = 0; i < 5; ++i) {
    inv *= 2 - p0 * inv;
  }
  return 0 - inv;
}

// Maps a value v + high * 2^(64 N) < 2p to its representative in [0, p).
template <std::size_t N>
constexpr auto reduce_once(std::array<u64, N> &v, u64 high, const std::array<u64, N> &p) noexcept
    -> void {
  std::array<u64, N> reduced = v;
  const u64 borrow = limbs::sub_assign(reduced, p);
  if (high != 0 || borrow == 0) {
    v = reduced;
  }
}

// 2^exponent mod p.
template <std::size_t N>
[[nodiscard]] constexpr auto pow2_mod(const std::array<u64, N> &p, std::size_t exponent) noexcept
    -> std::array<u64, N> {
  std::array<u64, N> x{};
  x[0] = 1;
  for (std::size_t i = 0; i < exponent; ++i) {
    const std::array<u64, N> addend = x;
    const u64 carry = limbs::add_assign(x, addend);
    reduce_once(x, carry, p);
  }
  return x;
}

// Montgomery multiplication a * b * R^-1 mod p (coarsely integrated operand scanning).
// Requires a * b < p * R, which holds when one operand is below p. The result is below p.
template <std::size_t N>
[[nodiscard]] constexpr auto mont_mul(const std::array<u64, N> &a, const std::array<u64, N> &b,
                                      const std::array<u64, N> &p, u64 n0) noexcept
    -> std::array<u64, N> {
  // The running value occupies N + 2 words: t, then t_hi, then t_top.
  std::array<u64, N> t{};
  u64 t_hi = 0;
  for (std::size_t i = 0; i < N; ++i) {
    // t += a * b[i]
    u64 carry = 0;
    for (std::size_t j = 0; j < N; ++j) {
      t[j] = limbs::mac(t[j], a[j], b[i], carry);
    }
    u64 t_top = 0;
    t_hi = limbs::adc(t_hi, carry, t_top);

    // t = (t + m * p) / 2^64, where m makes the lowest word vanish.
    const u64 m = t[0] * n0;
    carry = 0;
    static_cast<void>(limbs::mac(t[0], m, p[0], carry));
    for (std::size_t j = 1; j < N; ++j) {
      t[j - 1] = limbs::mac(t[j], m, p[j], carry);
    }
    u64 carry_out = 0;
    t[N - 1] = limbs::adc(t_hi, carry, carry_out);
    t_hi = t_top + carry_out;
  }
  reduce_once(t, t_hi, p);
  return t;
}

// p - 2 for p >= 3.
template <std::size_t N>
[[nodiscard]] constexpr auto minus_two(std::array<u64, N> p) noexcept -> std::array<u64, N> {
  std::array<u64, N> two{};
  two[0] = 2;
  static_cast<void>(limbs::sub_assign(p, two));
  return p;
}

} // namespace detail

template <typename Params> class PrimeField {
public:
  static constexpr std::size_t limb_count = Params::modulus.size();
  using Limbs = std::array<u64, limb_count>;

  static constexpr Limbs modulus = Params::modulus;
  // -modulus^-1 mod 2^64
  static constexpr u64 n0 = detail::neg_inv_word(modulus[0]);
  // R mod modulus, which is also the Montgomery form of 1.
  static constexpr Limbs r = detail::pow2_mod(modulus, 64 * limb_count);
  // R^2 mod modulus
  static constexpr Limbs r2 = detail::pow2_mod(modulus, 128 * limb_count);
  // modulus - 2, the exponent that inverts a non-zero element (Fermat's little theorem).
  static constexpr Limbs modulus_minus_two = detail::minus_two(modulus);

  static_assert((modulus[0] & 1U) == 1U, "modulus must be odd");
  static_assert(modulus[limb_count - 1] != 0, "top limb of the modulus must be non-zero");

  // Zero.
  constexpr PrimeField() noexcept = default;

  [[nodiscard]] static constexpr auto zero() noexcept -> PrimeField { return {}; }
  [[nodiscard]] static constexpr auto one() noexcept -> PrimeField { return PrimeField{r}; }

  // Builds the element for an integer given as little-endian limbs. Any value below R is
  // accepted and reduced modulo the modulus.
  [[nodiscard]] static constexpr auto from_limbs(const Limbs &value) noexcept -> PrimeField {
    return PrimeField{detail::mont_mul(value, r2, modulus, n0)};
  }

  [[nodiscard]] static constexpr auto from_u64(u64 value) noexcept -> PrimeField {
    Limbs wide{};
    wide[0] = value;
    return from_limbs(wide);
  }

  // The canonical integer in [0, modulus) as little-endian limbs.
  [[nodiscard]] constexpr auto to_limbs() const noexcept -> Limbs {
    Limbs unit{};
    unit[0] = 1;
    return detail::mont_mul(v_, unit, modulus, n0);
  }

  // The internal representation, value * R mod modulus.
  [[nodiscard]] constexpr auto montgomery_limbs() const noexcept -> const Limbs & { return v_; }

  [[nodiscard]] constexpr auto is_zero() const noexcept -> bool { return v_ == Limbs{}; }

  constexpr auto operator+=(const PrimeField &rhs) noexcept -> PrimeField & {
    const u64 carry = limbs::add_assign(v_, rhs.v_);
    detail::reduce_once(v_, carry, modulus);
    return *this;
  }

  constexpr auto operator-=(const PrimeField &rhs) noexcept -> PrimeField & {
    if (limbs::sub_assign(v_, rhs.v_) != 0) {
      // The difference wrapped below zero; adding the modulus wraps it back.
      static_cast<void>(limbs::add_assign(v_, modulus));
    }
    return *this;
  }

  constexpr auto operator*=(const PrimeField &rhs) noexcept -> PrimeField & {
    v_ = detail::mont_mul(v_, rhs.v_, modulus, n0);
    return *this;
  }

  [[nodiscard]] constexpr auto square() const noexcept -> PrimeField { return *this * *this; }

  // this^exponent, with the exponent given as little-endian limbs. The zeroth power is one.
  template <std::size_t M>
  [[nodiscard]] constexpr auto pow(const std::array<u64, M> &exponent) const noexcept
      -> PrimeField {
    PrimeField result = one();
    for (std::size_t i = M; i-- > 0;) {
      for (int bit = 63; bit >= 0; --bit) {
        result = result.square();
        if (((exponent[i] >> bit) & 1U) != 0) {
          result *= *this;
        }
      }
    }
    return result;
  }

  // The multiplicative inverse, or nothing for zero. Computed as this^(modulus - 2).
  [[nodiscard]] constexpr auto inverse() const noexcept -> std::optional<PrimeField> {
    if (is_zero()) {
      return std::nullopt;
    }
    return pow(modulus_minus_two);
  }

  [[nodiscard]] constexpr auto operator-() const noexcept -> PrimeField {
    return PrimeField{} - *this;
  }

  [[nodiscard]] friend constexpr auto operator+(PrimeField lhs, const PrimeField &rhs) noexcept
      -> PrimeField {
    return lhs += rhs;
  }

  [[nodiscard]] friend constexpr auto operator-(PrimeField lhs, const PrimeField &rhs) noexcept
      -> PrimeField {
    return lhs -= rhs;
  }

  [[nodiscard]] friend constexpr auto operator*(PrimeField lhs, const PrimeField &rhs) noexcept
      -> PrimeField {
    return lhs *= rhs;
  }

  // Montgomery forms are kept in [0, modulus), so limb equality is field equality.
  [[nodiscard]] friend constexpr auto operator==(const PrimeField &, const PrimeField &) noexcept
      -> bool = default;

private:
  constexpr explicit PrimeField(const Limbs &montgomery) noexcept : v_{montgomery} {}

  Limbs v_{};
};

} // namespace tacit
