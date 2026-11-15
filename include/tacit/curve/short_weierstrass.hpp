#pragma once

#include <tacit/limbs.hpp>

#include <array>
#include <cstddef>

// Points on a short Weierstrass curve y^2 = x^3 + b, the a = 0 case that covers the BN and
// BLS12 families.
//
// A curve is described by a parameter type:
//
//     struct MyParams {
//       using Field = ...;  // coordinate field
//       using Scalar = ...; // prime field of the group order
//       static constexpr Field b{...};
//       static constexpr Field generator_x{...};
//       static constexpr Field generator_y{...};
//     };
//
// AffinePoint is the compact form for storage and input. JacobianPoint is the working form:
// its group operations need no field inversion.

namespace tacit {

template <typename Params> struct AffinePoint {
  using Field = typename Params::Field;

  Field x{};
  Field y{};
  // When set, this is the point at infinity and x and y carry no meaning.
  bool infinity = true;

  // The point at infinity.
  constexpr AffinePoint() noexcept = default;
  // The point (px, py). It is not checked to lie on the curve; see is_on_curve.
  constexpr AffinePoint(const Field &px, const Field &py) noexcept
      : x{px}, y{py}, infinity{false} {}

  [[nodiscard]] static constexpr auto identity() noexcept -> AffinePoint { return {}; }
  [[nodiscard]] static constexpr auto generator() noexcept -> AffinePoint {
    return {Params::generator_x, Params::generator_y};
  }

  [[nodiscard]] constexpr auto is_identity() const noexcept -> bool { return infinity; }

  [[nodiscard]] constexpr auto is_on_curve() const noexcept -> bool {
    return infinity || y.square() == x.square() * x + Params::b;
  }

  [[nodiscard]] constexpr auto operator-() const noexcept -> AffinePoint {
    return infinity ? *this : AffinePoint{x, -y};
  }

  [[nodiscard]] friend constexpr auto operator==(const AffinePoint &p,
                                                 const AffinePoint &q) noexcept -> bool {
    if (p.infinity || q.infinity) {
      return p.infinity == q.infinity;
    }
    return p.x == q.x && p.y == q.y;
  }
};

template <typename Params> class JacobianPoint {
public:
  using Field = typename Params::Field;
  using Scalar = typename Params::Scalar;
  using Affine = AffinePoint<Params>;

  // (x, y, z) stands for the affine point (x / z^2, y / z^3). z = 0 is the point at infinity.
  Field x{Field::one()};
  Field y{Field::one()};
  Field z{};

  // The point at infinity.
  constexpr JacobianPoint() noexcept = default;
  constexpr JacobianPoint(const Field &px, const Field &py, const Field &pz) noexcept
      : x{px}, y{py}, z{pz} {}

  [[nodiscard]] static constexpr auto identity() noexcept -> JacobianPoint { return {}; }
  [[nodiscard]] static constexpr auto generator() noexcept -> JacobianPoint {
    return from_affine(Affine::generator());
  }

  [[nodiscard]] static constexpr auto from_affine(const Affine &p) noexcept -> JacobianPoint {
    return p.infinity ? JacobianPoint{} : JacobianPoint{p.x, p.y, Field::one()};
  }

  // Costs one field inversion.
  [[nodiscard]] constexpr auto to_affine() const noexcept -> Affine {
    if (is_identity()) {
      return {};
    }
    const Field z_inv = *z.inverse();
    const Field z_inv2 = z_inv.square();
    return {x * z_inv2, y * z_inv2 * z_inv};
  }

  [[nodiscard]] constexpr auto is_identity() const noexcept -> bool { return z.is_zero(); }

  // y^2 = x^3 + b z^6, the curve equation scaled by z^6.
  [[nodiscard]] constexpr auto is_on_curve() const noexcept -> bool {
    if (is_identity()) {
      return true;
    }
    const Field z2 = z.square();
    const Field z6 = z2.square() * z2;
    return y.square() == x.square() * x + Params::b * z6;
  }

  // dbl-2009-l: 2M + 5S.
  [[nodiscard]] constexpr auto doubled() const noexcept -> JacobianPoint {
    if (is_identity()) {
      return *this;
    }
    const Field a = x.square();
    const Field b = y.square();
    const Field c = b.square();
    const Field d = twice((x + b).square() - a - c);
    const Field e = a + twice(a);
    const Field x3 = e.square() - twice(d);
    const Field c8 = twice(twice(twice(c)));
    return {x3, e * (d - x3) - c8, twice(y * z)};
  }

  // add-2007-bl: 11M + 5S.
  constexpr auto operator+=(const JacobianPoint &rhs) noexcept -> JacobianPoint & {
    if (rhs.is_identity()) {
      return *this;
    }
    if (is_identity()) {
      return *this = rhs;
    }
    const Field z1z1 = z.square();
    const Field z2z2 = rhs.z.square();
    const Field u1 = x * z2z2;
    const Field u2 = rhs.x * z1z1;
    const Field s1 = y * rhs.z * z2z2;
    const Field s2 = rhs.y * z * z1z1;
    const Field h = u2 - u1;
    const Field r = twice(s2 - s1);
    if (h.is_zero()) {
      // Same x coordinate: either the same point or its negation.
      return *this = r.is_zero() ? doubled() : JacobianPoint{};
    }
    const Field i = twice(h).square();
    const Field j = h * i;
    const Field v = u1 * i;
    const Field x3 = r.square() - j - twice(v);
    const Field y3 = r * (v - x3) - twice(s1 * j);
    const Field z3 = ((z + rhs.z).square() - z1z1 - z2z2) * h;
    x = x3;
    y = y3;
    z = z3;
    return *this;
  }

  // Mixed addition with an affine point, madd-2007-bl: 7M + 4S.
  constexpr auto operator+=(const Affine &rhs) noexcept -> JacobianPoint & {
    if (rhs.infinity) {
      return *this;
    }
    if (is_identity()) {
      return *this = from_affine(rhs);
    }
    const Field z1z1 = z.square();
    const Field u2 = rhs.x * z1z1;
    const Field s2 = rhs.y * z * z1z1;
    const Field h = u2 - x;
    const Field r = twice(s2 - y);
    if (h.is_zero()) {
      return *this = r.is_zero() ? doubled() : JacobianPoint{};
    }
    const Field hh = h.square();
    const Field i = twice(twice(hh));
    const Field j = h * i;
    const Field v = x * i;
    const Field x3 = r.square() - j - twice(v);
    const Field y3 = r * (v - x3) - twice(y * j);
    const Field z3 = (z + h).square() - z1z1 - hh;
    x = x3;
    y = y3;
    z = z3;
    return *this;
  }

  constexpr auto operator-=(const JacobianPoint &rhs) noexcept -> JacobianPoint & {
    return *this += -rhs;
  }

  [[nodiscard]] constexpr auto operator-() const noexcept -> JacobianPoint { return {x, -y, z}; }

  // this * k for an integer k given as little-endian limbs, by double-and-add from the top
  // bit. Any k is accepted, including multiples of the group order.
  template <std::size_t M>
  [[nodiscard]] constexpr auto mul(const std::array<u64, M> &k) const noexcept -> JacobianPoint {
    JacobianPoint result{};
    for (std::size_t i = M; i-- > 0;) {
      for (int bit = 63; bit >= 0; --bit) {
        result = result.doubled();
        if (((k[i] >> bit) & 1U) != 0) {
          result += *this;
        }
      }
    }
    return result;
  }

  [[nodiscard]] friend constexpr auto operator+(JacobianPoint p, const JacobianPoint &q) noexcept
      -> JacobianPoint {
    return p += q;
  }

  [[nodiscard]] friend constexpr auto operator+(JacobianPoint p, const Affine &q) noexcept
      -> JacobianPoint {
    return p += q;
  }

  [[nodiscard]] friend constexpr auto operator-(JacobianPoint p, const JacobianPoint &q) noexcept
      -> JacobianPoint {
    return p -= q;
  }

  [[nodiscard]] friend constexpr auto operator*(const JacobianPoint &p, const Scalar &k) noexcept
      -> JacobianPoint {
    return p.mul(k.to_limbs());
  }

  // Equality of the represented points: x1 z2^2 = x2 z1^2 and y1 z2^3 = y2 z1^3.
  [[nodiscard]] friend constexpr auto operator==(const JacobianPoint &p,
                                                 const JacobianPoint &q) noexcept -> bool {
    if (p.is_identity() || q.is_identity()) {
      return p.is_identity() == q.is_identity();
    }
    const Field z1z1 = p.z.square();
    const Field z2z2 = q.z.square();
    return p.x * z2z2 == q.x * z1z1 && p.y * z2z2 * q.z == q.y * z1z1 * p.z;
  }

private:
  [[nodiscard]] static constexpr auto twice(const Field &a) noexcept -> Field { return a + a; }
};

} // namespace tacit
