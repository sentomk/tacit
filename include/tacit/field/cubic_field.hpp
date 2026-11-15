#pragma once

#include <optional>

// Cubic extension Base[v] / (v^3 - xi). An element is c0 + c1 * v + c2 * v^2.
//
// The parameter type names the base field and the cubic non-residue xi:
//
//     struct MyParams {
//       using Base = ...;
//       static constexpr Base nonresidue{...}; // xi
//       static constexpr auto mul_by_nonresidue(const Base &x) noexcept -> Base; // xi * x
//     };
//     using MyField = tacit::CubicField<MyParams>;

namespace tacit {

template <typename Params> class CubicField {
public:
  using Base = typename Params::Base;

  Base c0{};
  Base c1{};
  Base c2{};

  // Zero.
  constexpr CubicField() noexcept = default;
  constexpr CubicField(const Base &a0, const Base &a1, const Base &a2) noexcept
      : c0{a0}, c1{a1}, c2{a2} {}

  [[nodiscard]] static constexpr auto zero() noexcept -> CubicField { return {}; }
  [[nodiscard]] static constexpr auto one() noexcept -> CubicField {
    return {Base::one(), Base::zero(), Base::zero()};
  }

  [[nodiscard]] constexpr auto is_zero() const noexcept -> bool {
    return c0.is_zero() && c1.is_zero() && c2.is_zero();
  }

  constexpr auto operator+=(const CubicField &rhs) noexcept -> CubicField & {
    c0 += rhs.c0;
    c1 += rhs.c1;
    c2 += rhs.c2;
    return *this;
  }

  constexpr auto operator-=(const CubicField &rhs) noexcept -> CubicField & {
    c0 -= rhs.c0;
    c1 -= rhs.c1;
    c2 -= rhs.c2;
    return *this;
  }

  // Karatsuba: six base multiplications instead of nine. With v_i = a_i b_i,
  //   c0 = v0 + xi ((a1 + a2)(b1 + b2) - v1 - v2)
  //   c1 = (a0 + a1)(b0 + b1) - v0 - v1 + xi v2
  //   c2 = (a0 + a2)(b0 + b2) - v0 - v2 + v1
  constexpr auto operator*=(const CubicField &rhs) noexcept -> CubicField & {
    const Base v0 = c0 * rhs.c0;
    const Base v1 = c1 * rhs.c1;
    const Base v2 = c2 * rhs.c2;
    const Base t0 = (c1 + c2) * (rhs.c1 + rhs.c2) - v1 - v2;
    const Base t1 = (c0 + c1) * (rhs.c0 + rhs.c1) - v0 - v1;
    const Base t2 = (c0 + c2) * (rhs.c0 + rhs.c2) - v0 - v2;
    c0 = v0 + xi(t0);
    c1 = t1 + xi(v2);
    c2 = t2 + v1;
    return *this;
  }

  // Scales all three coefficients by an element of the base field.
  constexpr auto operator*=(const Base &scalar) noexcept -> CubicField & {
    const Base s = scalar; // scalar may refer to a coefficient of this element
    c0 *= s;
    c1 *= s;
    c2 *= s;
    return *this;
  }

  // Chung-Hasan squaring: two base multiplications and three base squarings. With
  //   s0 = a0^2, s1 = 2 a0 a1, s2 = (a0 - a1 + a2)^2, s3 = 2 a1 a2, s4 = a2^2,
  //   c0 = s0 + xi s3
  //   c1 = s1 + xi s4
  //   c2 = s1 + s2 + s3 - s0 - s4
  [[nodiscard]] constexpr auto square() const noexcept -> CubicField {
    const Base s0 = c0.square();
    const Base a0a1 = c0 * c1;
    const Base s1 = a0a1 + a0a1;
    const Base s2 = (c0 - c1 + c2).square();
    const Base a1a2 = c1 * c2;
    const Base s3 = a1a2 + a1a2;
    const Base s4 = c2.square();
    return {s0 + xi(s3), s1 + xi(s4), s1 + s2 + s3 - s0 - s4};
  }

  // Multiplication by v: (c0 + c1 v + c2 v^2) v = xi c2 + c0 v + c1 v^2.
  [[nodiscard]] constexpr auto mul_by_v() const noexcept -> CubicField { return {xi(c2), c0, c1}; }

  // The multiplicative inverse, or nothing for zero. With
  //   t0 = a0^2 - xi a1 a2,  t1 = xi a2^2 - a0 a1,  t2 = a1^2 - a0 a2,
  // the norm down to the base field is d = a0 t0 + xi (a2 t1 + a1 t2), and the inverse is
  // (t0 + t1 v + t2 v^2) / d.
  [[nodiscard]] constexpr auto inverse() const noexcept -> std::optional<CubicField> {
    const Base t0 = c0.square() - xi(c1 * c2);
    const Base t1 = xi(c2.square()) - c0 * c1;
    const Base t2 = c1.square() - c0 * c2;
    const auto d_inverse = (c0 * t0 + xi(c2 * t1 + c1 * t2)).inverse();
    if (!d_inverse) {
      return std::nullopt;
    }
    return CubicField{t0 * *d_inverse, t1 * *d_inverse, t2 * *d_inverse};
  }

  [[nodiscard]] constexpr auto operator-() const noexcept -> CubicField { return {-c0, -c1, -c2}; }

  [[nodiscard]] friend constexpr auto operator+(CubicField lhs, const CubicField &rhs) noexcept
      -> CubicField {
    return lhs += rhs;
  }

  [[nodiscard]] friend constexpr auto operator-(CubicField lhs, const CubicField &rhs) noexcept
      -> CubicField {
    return lhs -= rhs;
  }

  [[nodiscard]] friend constexpr auto operator*(CubicField lhs, const CubicField &rhs) noexcept
      -> CubicField {
    return lhs *= rhs;
  }

  [[nodiscard]] friend constexpr auto operator*(CubicField lhs, const Base &scalar) noexcept
      -> CubicField {
    return lhs *= scalar;
  }

  [[nodiscard]] friend constexpr auto operator==(const CubicField &, const CubicField &) noexcept
      -> bool = default;

private:
  [[nodiscard]] static constexpr auto xi(const Base &x) noexcept -> Base {
    return Params::mul_by_nonresidue(x);
  }
};

} // namespace tacit
