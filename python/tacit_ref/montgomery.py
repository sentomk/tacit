"""Reference Montgomery arithmetic on plain Python integers.

The word-by-word reduction follows the structure the C++ implementation will use,
so it can serve as the oracle for limb-level tests later.
"""

LIMB_BITS = 64
WORD = 1 << LIMB_BITS


def limb_count(p: int) -> int:
    return (p.bit_length() + LIMB_BITS - 1) // LIMB_BITS


class Montgomery:
    """Montgomery form modulo an odd p, with R = 2^(64 * limb_count)."""

    def __init__(self, p: int) -> None:
        if p < 3 or p % 2 == 0:
            raise ValueError("modulus must be odd and greater than 2")
        self.p = p
        self.limbs = limb_count(p)
        self.n0 = -pow(p, -1, WORD) % WORD  # -p^-1 mod 2^64
        self.r = pow(2, LIMB_BITS * self.limbs, p)  # R mod p
        self.r2 = self.r * self.r % p  # R^2 mod p

    def redc(self, t: int) -> int:
        """Word-by-word reduction: t * R^-1 mod p for 0 <= t < p * R."""
        for _ in range(self.limbs):
            m = (t % WORD) * self.n0 % WORD
            t = (t + m * self.p) >> LIMB_BITS
        return t - self.p if t >= self.p else t

    def to_mont(self, a: int) -> int:
        return self.redc(a * self.r2)

    def from_mont(self, a: int) -> int:
        return self.redc(a)

    def mul(self, a: int, b: int) -> int:
        return self.redc(a * b)
