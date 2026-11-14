#pragma once

#include <tacit/field/prime_field.hpp>

// Quadratic extension Base[u] / (u^2 + 1). An element is c0 + c1 * u.

namespace tacit {

template <typename Base> class QuadraticField {
public:
  // u^2 + 1 is irreducible over a prime field exactly when the modulus is 3 mod 4.
  static_assert((Base::modulus[0] & 3U) == 3U, "u^2 + 1 must be irreducible over the base field");

  Base c0{};
  Base c1{};

  // Zero.
  constexpr QuadraticField() noexcept = default;
  constexpr QuadraticField(const Base &real, const Base &imag) noexcept : c0{real}, c1{imag} {}

  [[nodiscard]] static constexpr auto zero() noexcept -> QuadraticField { return {}; }
  [[nodiscard]] static constexpr auto one() noexcept -> QuadraticField {
    return {Base::one(), Base::zero()};
  }

  [[nodiscard]] constexpr auto is_zero() const noexcept -> bool {
    return c0.is_zero() && c1.is_zero();
  }

  constexpr auto operator+=(const QuadraticField &rhs) noexcept -> QuadraticField & {
    c0 += rhs.c0;
    c1 += rhs.c1;
    return *this;
  }

  constexpr auto operator-=(const QuadraticField &rhs) noexcept -> QuadraticField & {
    c0 -= rhs.c0;
    c1 -= rhs.c1;
    return *this;
  }

  // Karatsuba: three base multiplications instead of four.
  // (a0 + a1 u)(b0 + b1 u) = (a0 b0 - a1 b1) + ((a0 + a1)(b0 + b1) - a0 b0 - a1 b1) u
  constexpr auto operator*=(const QuadraticField &rhs) noexcept -> QuadraticField & {
    const Base v0 = c0 * rhs.c0;
    const Base v1 = c1 * rhs.c1;
    const Base cross = (c0 + c1) * (rhs.c0 + rhs.c1);
    c0 = v0 - v1;
    c1 = cross - v0 - v1;
    return *this;
  }

  // Scales both coefficients by an element of the base field.
  constexpr auto operator*=(const Base &scalar) noexcept -> QuadraticField & {
    const Base s = scalar; // scalar may refer to c0 or c1 of this element
    c0 *= s;
    c1 *= s;
    return *this;
  }

  // Complex squaring: two base multiplications.
  // (a0 + a1 u) ^ 2 = (a0 + a1)(a0 - a1) + 2 a0 a1 u
  [[nodiscard]] constexpr auto square() const noexcept -> QuadraticField {
    const Base product = c0 * c1;
    return {(c0 + c1) * (c0 - c1), product + product};
  }

  // c0 - c1 u, which is also the Frobenius map x -> x^p.
  [[nodiscard]] constexpr auto conjugate() const noexcept -> QuadraticField { return {c0, -c1}; }

  // c0^2 + c1^2, an element of the base field.
  [[nodiscard]] constexpr auto norm() const noexcept -> Base { return c0.square() + c1.square(); }

  [[nodiscard]] constexpr auto operator-() const noexcept -> QuadraticField { return {-c0, -c1}; }

  [[nodiscard]] friend constexpr auto operator+(QuadraticField lhs,
                                                const QuadraticField &rhs) noexcept
      -> QuadraticField {
    return lhs += rhs;
  }

  [[nodiscard]] friend constexpr auto operator-(QuadraticField lhs,
                                                const QuadraticField &rhs) noexcept
      -> QuadraticField {
    return lhs -= rhs;
  }

  [[nodiscard]] friend constexpr auto operator*(QuadraticField lhs,
                                                const QuadraticField &rhs) noexcept
      -> QuadraticField {
    return lhs *= rhs;
  }

  [[nodiscard]] friend constexpr auto operator*(QuadraticField lhs, const Base &scalar) noexcept
      -> QuadraticField {
    return lhs *= scalar;
  }

  [[nodiscard]] friend constexpr auto operator==(const QuadraticField &,
                                                 const QuadraticField &) noexcept -> bool = default;
};

} // namespace tacit
