"""Number-theory helpers for the reference models, on plain Python integers."""

# Fixed Miller-Rabin bases. The result is probabilistic for 254-bit moduli, which is
# enough to cross-check the proven primality test done in Sage.
_MR_BASES = (2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47, 53, 59, 61, 67, 71)


def is_probable_prime(n: int) -> bool:
    if n < 2:
        return False
    for q in _MR_BASES:
        if n % q == 0:
            return n == q

    d, s = n - 1, 0
    while d % 2 == 0:
        d //= 2
        s += 1

    for a in _MR_BASES:
        x = pow(a, d, n)
        if x in (1, n - 1):
            continue
        for _ in range(s - 1):
            x = x * x % n
            if x == n - 1:
                break
        else:
            return False
    return True


def two_adicity(p: int) -> tuple[int, int]:
    """Return (s, t) with p - 1 = 2^s * t and t odd."""
    m = p - 1
    s = (m & -m).bit_length() - 1
    return s, m >> s


def smallest_non_residue(p: int) -> int:
    """Smallest c >= 2 that is a quadratic non-residue modulo the odd prime p."""
    c = 2
    while pow(c, (p - 1) // 2, p) != p - 1:  # Euler's criterion
        c += 1
    return c
