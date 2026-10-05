"""Generate Montgomery and NTT constants for the BN254 fields Fp and Fr.

Run from the repository root:

    micromamba run -n sage sage scripts/generate_constants.sage

Writes tests/vectors/bn254_constants.json, which the Python and C++ tests read.
"""

import json
from pathlib import Path

LIMB_BITS = 64
WORD = 2 ^ LIMB_BITS
OUT = Path("tests/vectors/bn254_constants.json")

# BN254 (alt_bn128) curve parameter. p and r are derived from it and compared
# with the literal moduli, so two independent sources check each other.
X = 4965661367192848881
P_LITERAL = 21888242871839275222246405745257275088696311157297823662689037894645226208583
R_LITERAL = 21888242871839275222246405745257275088548364400416034343698204186575808495617
SOURCE = "Ethereum alt_bn128 (EIP-196/197); also derived from curve parameter x"


def limbs_le(n, count):
    """Little-endian 64-bit limbs of n as hex strings."""
    n = int(n)
    mask = int(WORD) - 1
    return [hex((n >> (LIMB_BITS * i)) & mask) for i in range(count)]


def montgomery_redc(t, p, n0, limbs):
    """Word-by-word Montgomery reduction: t * R^-1 mod p, for 0 <= t < p * R."""
    for _ in range(limbs):
        m = ((t % WORD) * n0) % WORD
        t = (t + m * p) >> LIMB_BITS
    return t - p if t >= p else t


def check_montgomery(p, limbs, r2, n0, samples=64):
    """Check R^2 and n0 by running a Montgomery multiplication on random inputs."""
    set_random_seed(0)
    for _ in range(samples):
        a = ZZ.random_element(p)
        b = ZZ.random_element(p)
        a_m = montgomery_redc(a * r2, p, n0, limbs)  # a * R mod p
        b_m = montgomery_redc(b * r2, p, n0, limbs)  # b * R mod p
        prod_m = montgomery_redc(a_m * b_m, p, n0, limbs)  # a * b * R mod p
        assert montgomery_redc(prod_m, p, n0, limbs) == (a * b) % p


def field_constants(name, role, modulus):
    p = ZZ(modulus)
    assert p.is_prime(), f"{name}: modulus is not prime"

    bits = p.nbits()
    limbs = (bits + LIMB_BITS - 1) // LIMB_BITS
    R = 2 ^ (LIMB_BITS * limbs)
    r_mod_p = R % p
    r2_mod_p = (R * R) % p
    n0 = (-p.inverse_mod(WORD)) % WORD
    assert (n0 * p + 1) % WORD == 0, f"{name}: n0 * p != -1 mod 2^{LIMB_BITS}"
    check_montgomery(p, limbs, r2_mod_p, n0)

    # p - 1 = 2^s * t with t odd. Raising a quadratic non-residue to t gives an
    # element of exact order 2^s.
    s = (p - 1).valuation(2)
    t = (p - 1) >> s
    F = GF(p)
    c = 2
    while F(c).is_square():
        c += 1
    root = F(c) ^ t

    roots = []  # roots[k] has exact order 2^k
    for k in range(s + 1):
        w = root ^ (2 ^ (s - k))
        assert w ^ (2 ^ k) == 1
        assert k == 0 or w ^ (2 ^ (k - 1)) == -1
        roots.append(hex(int(w.lift())))

    return {
        "role": role,
        "source": SOURCE,
        "modulus": hex(int(p)),
        "bits": int(bits),
        "limbs": int(limbs),
        "modulus_limbs": limbs_le(p, limbs),
        "r_mod_p": hex(int(r_mod_p)),
        "r2_mod_p": hex(int(r2_mod_p)),
        "n0": hex(int(n0)),
        "two_adicity": int(s),
        "odd_part": hex(int(t)),
        "nonresidue": int(c),
        "root_of_unity": roots[-1],
        "two_adic_roots": roots,
    }


def main():
    p = 36 * X ^ 4 + 36 * X ^ 3 + 24 * X ^ 2 + 6 * X + 1
    r = 36 * X ^ 4 + 36 * X ^ 3 + 18 * X ^ 2 + 6 * X + 1
    assert p == P_LITERAL and r == R_LITERAL, "curve parameter and literal moduli disagree"

    if not OUT.parent.is_dir():
        raise SystemExit(f"{OUT.parent} not found; run from the repository root")

    data = {
        "curve": "bn254",
        "limb_bits": int(LIMB_BITS),
        "curve_param_x": hex(int(X)),
        "fields": {
            "Fp": field_constants("Fp", "base field of the curve coordinates", p),
            "Fr": field_constants("Fr", "scalar field / circuit field", r),
        },
    }
    OUT.write_text(json.dumps(data, indent=2) + "\n")
    print(f"wrote {OUT}")


main()
