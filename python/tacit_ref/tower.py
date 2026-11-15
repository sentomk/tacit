"""Reference arithmetic for the extension tower over a prime field, on plain integers.

    Fp2 = Fp[u] / (u^2 + 1)      an element is a pair (c0, c1)
    Fp6 = Fp2[v] / (v^3 - xi)    an element is a triple of Fp2 elements

Every product uses the schoolbook formula, so the results do not depend on the Karatsuba
and Chung-Hasan formulas of the C++ implementation.
"""

Fp2 = tuple[int, int]
Fp6 = tuple[Fp2, Fp2, Fp2]

FP2_ZERO: Fp2 = (0, 0)
FP2_ONE: Fp2 = (1, 0)


class Tower:
    def __init__(self, p: int, xi: Fp2) -> None:
        if p % 4 != 3:
            raise ValueError("u^2 + 1 is irreducible only for p = 3 mod 4")
        self.p = p
        self.xi = xi

    # Fp2

    def fp2_add(self, a: Fp2, b: Fp2) -> Fp2:
        return ((a[0] + b[0]) % self.p, (a[1] + b[1]) % self.p)

    def fp2_sub(self, a: Fp2, b: Fp2) -> Fp2:
        return ((a[0] - b[0]) % self.p, (a[1] - b[1]) % self.p)

    def fp2_neg(self, a: Fp2) -> Fp2:
        return (-a[0] % self.p, -a[1] % self.p)

    def fp2_mul(self, a: Fp2, b: Fp2) -> Fp2:
        return ((a[0] * b[0] - a[1] * b[1]) % self.p, (a[0] * b[1] + a[1] * b[0]) % self.p)

    def fp2_conjugate(self, a: Fp2) -> Fp2:
        return (a[0], -a[1] % self.p)

    def fp2_norm(self, a: Fp2) -> int:
        return (a[0] * a[0] + a[1] * a[1]) % self.p

    def fp2_pow(self, a: Fp2, e: int) -> Fp2:
        result = FP2_ONE
        while e:
            if e & 1:
                result = self.fp2_mul(result, a)
            a = self.fp2_mul(a, a)
            e >>= 1
        return result

    # Fp6

    def fp6_add(self, a: Fp6, b: Fp6) -> Fp6:
        return tuple(self.fp2_add(x, y) for x, y in zip(a, b))

    def fp6_sub(self, a: Fp6, b: Fp6) -> Fp6:
        return tuple(self.fp2_sub(x, y) for x, y in zip(a, b))

    def fp6_neg(self, a: Fp6) -> Fp6:
        return tuple(self.fp2_neg(x) for x in a)

    def fp6_mul(self, a: Fp6, b: Fp6) -> Fp6:
        # Multiply as polynomials in v, then fold v^3 and v^4 back with v^3 = xi.
        d = [FP2_ZERO] * 5
        for i in range(3):
            for j in range(3):
                d[i + j] = self.fp2_add(d[i + j], self.fp2_mul(a[i], b[j]))
        return (
            self.fp2_add(d[0], self.fp2_mul(self.xi, d[3])),
            self.fp2_add(d[1], self.fp2_mul(self.xi, d[4])),
            d[2],
        )

    def fp6_scale(self, a: Fp6, s: Fp2) -> Fp6:
        return tuple(self.fp2_mul(x, s) for x in a)

    def fp6_mul_by_v(self, a: Fp6) -> Fp6:
        return (self.fp2_mul(self.xi, a[2]), a[0], a[1])
