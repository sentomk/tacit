#pragma once

// Quadratic extension Base[w] / (w^2 - beta) for a general non-residue beta. An element is
// c0 + c1 * w. QuadraticField in quadratic_field.hpp covers the special case beta = -1 over
// a prime field with shorter formulas.
//
// The parameter type names the base field and the quadratic non-residue beta:
//
//     struct MyParams {
//       using Base = ...;
//       static constexpr Base nonresidue{...}; // beta
//       static constexpr auto mul_by_nonresidue(const Base &x) noexcept -> Base; // beta * x
//     };
//     using MyField = tacit::QuadraticExtension<MyParams>;

namespace tacit {

template <typename Params> class QuadraticExtension {
public:
  using Base = typename Params::Base;

  Base c0{};
  Base c1{};

  // Zero.
  constexpr QuadraticExtension() noexcept = default;
  constexpr QuadraticExtension(const Base &a0, const Base &a1) noexcept : c0{a0}, c1{a1} {}

  [[nodiscard]] static constexpr auto zero() noexcept -> QuadraticExtension { return {}; }
  [[nodiscard]] static constexpr auto one() noexcept -> QuadraticExtension {
    return {Base::one(), Base::zero()};
  }

  [[nodiscard]] constexpr auto is_zero() const noexcept -> bool {
    return c0.is_zero() && c1.is_zero();
  }

  constexpr auto operator+=(const QuadraticExtension &rhs) noexcept -> QuadraticExtension & {
    c0 += rhs.c0;
    c1 += rhs.c1;
    return *this;
  }

  constexpr auto operator-=(const QuadraticExtension &rhs) noexcept -> QuadraticExtension & {
    c0 -= rhs.c0;
    c1 -= rhs.c1;
    return *this;
  }

  // Karatsuba: three base multiplications instead of four. With v_i = a_i b_i,
  //   c0 = v0 + beta v1
  //   c1 = (a0 + a1)(b0 + b1) - v0 - v1
  constexpr auto operator*=(const QuadraticExtension &rhs) noexcept -> QuadraticExtension & {
    const Base v0 = c0 * rhs.c0;
    const Base v1 = c1 * rhs.c1;
    const Base cross = (c0 + c1) * (rhs.c0 + rhs.c1);
    c0 = v0 + beta(v1);
    c1 = cross - v0 - v1;
    return *this;
  }

  // Scales both coefficients by an element of the base field.
  constexpr auto operator*=(const Base &scalar) noexcept -> QuadraticExtension & {
    const Base s = scalar; // scalar may refer to c0 or c1 of this element
    c0 *= s;
    c1 *= s;
    return *this;
  }

  // Complex squaring: two base multiplications. With t = a0 a1,
  //   c0 = (a0 + a1)(a0 + beta a1) - t - beta t
  //   c1 = 2 t
  [[nodiscard]] constexpr auto square() const noexcept -> QuadraticExtension {
    const Base t = c0 * c1;
    return {(c0 + c1) * (c0 + beta(c1)) - t - beta(t), t + t};
  }

  // c0 - c1 w, the image under the non-trivial automorphism over the base field.
  [[nodiscard]] constexpr auto conjugate() const noexcept -> QuadraticExtension {
    return {c0, -c1};
  }

  [[nodiscard]] constexpr auto operator-() const noexcept -> QuadraticExtension {
    return {-c0, -c1};
  }

  [[nodiscard]] friend constexpr auto operator+(QuadraticExtension lhs,
                                                const QuadraticExtension &rhs) noexcept
      -> QuadraticExtension {
    return lhs += rhs;
  }

  [[nodiscard]] friend constexpr auto operator-(QuadraticExtension lhs,
                                                const QuadraticExtension &rhs) noexcept
      -> QuadraticExtension {
    return lhs -= rhs;
  }

  [[nodiscard]] friend constexpr auto operator*(QuadraticExtension lhs,
                                                const QuadraticExtension &rhs) noexcept
      -> QuadraticExtension {
    return lhs *= rhs;
  }

  [[nodiscard]] friend constexpr auto operator*(QuadraticExtension lhs, const Base &scalar) noexcept
      -> QuadraticExtension {
    return lhs *= scalar;
  }

  [[nodiscard]] friend constexpr auto operator==(const QuadraticExtension &,
                                                 const QuadraticExtension &) noexcept
      -> bool = default;

private:
  [[nodiscard]] static constexpr auto beta(const Base &x) noexcept -> Base {
    return Params::mul_by_nonresidue(x);
  }
};

} // namespace tacit
