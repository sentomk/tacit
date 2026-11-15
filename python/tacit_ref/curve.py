"""Reference arithmetic on a short Weierstrass curve y^2 = x^3 + b over a prime field.

Points are affine pairs (x, y) of plain integers, with None for the point at infinity.
Addition uses the chord-and-tangent formulas with a field inversion per operation, so the
results do not depend on the Jacobian formulas of the C++ implementation.
"""

Point = tuple[int, int] | None


class Curve:
    def __init__(self, p: int, b: int) -> None:
        self.p = p
        self.b = b % p

    def is_on_curve(self, pt: Point) -> bool:
        if pt is None:
            return True
        x, y = pt
        return 0 <= x < self.p and 0 <= y < self.p and (y * y - x * x * x - self.b) % self.p == 0

    def neg(self, pt: Point) -> Point:
        if pt is None:
            return None
        return (pt[0], -pt[1] % self.p)

    def add(self, a: Point, b: Point) -> Point:
        if a is None:
            return b
        if b is None:
            return a
        p = self.p
        (x1, y1), (x2, y2) = a, b
        if x1 == x2:
            if (y1 + y2) % p == 0:
                return None
            slope = 3 * x1 * x1 * pow(2 * y1, -1, p) % p
        else:
            slope = (y2 - y1) * pow(x2 - x1, -1, p) % p
        x3 = (slope * slope - x1 - x2) % p
        return (x3, (slope * (x1 - x3) - y1) % p)

    def mul(self, pt: Point, k: int) -> Point:
        if k < 0:
            raise ValueError("scalar must be non-negative")
        result: Point = None
        while k:
            if k & 1:
                result = self.add(result, pt)
            pt = self.add(pt, pt)
            k >>= 1
        return result
