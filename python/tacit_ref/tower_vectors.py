"""Generate tests/vectors/bn254_tower_ops.json: test vectors for BN254 Fp2, Fp6 and Fp12.

Expected values come from the schoolbook arithmetic in tacit_ref.tower. The base field
modulus is read from tests/vectors/bn254_constants.json. The output is deterministic, so
running this again reproduces the committed file byte for byte:

    python -m tacit_ref.tower_vectors
"""

import json
import random
from pathlib import Path

from tacit_ref.tower import FP6_ONE, FP6_ZERO, Fp2, Fp6, Fp12, Tower

VECTORS_DIR = Path(__file__).resolve().parents[2] / "tests" / "vectors"
CONSTANTS = VECTORS_DIR / "bn254_constants.json"
OUTPUT = VECTORS_DIR / "bn254_tower_ops.json"

SEED = "tacit/bn254/tower-ops/v1"
XI: Fp2 = (9, 1)  # Fp6 = Fp2[v] / (v^3 - xi)
RANDOM_FP2_CASES = 32
RANDOM_FP6_CASES = 16
RANDOM_FP12_CASES = 8


def enc2(a: Fp2) -> list[str]:
    return [hex(a[0]), hex(a[1])]


def enc6(a: Fp6) -> list[list[str]]:
    return [enc2(c) for c in a]


def enc12(a: Fp12) -> list[list[list[str]]]:
    return [enc6(c) for c in a]


def fp2_vectors(tower: Tower, rng: random.Random) -> dict:
    p = tower.p
    special = [(0, 0), (1, 0), (0, 1), (p - 1, 0), (p - 1, p - 1)]
    pairs = [(a, b) for a in special for b in special]
    for _ in range(RANDOM_FP2_CASES):
        a = (rng.randrange(p), rng.randrange(p))
        b = (rng.randrange(p), rng.randrange(p))
        pairs.append((a, b))

    return {
        "ops": [
            {
                "a": enc2(a),
                "b": enc2(b),
                "add": enc2(tower.fp2_add(a, b)),
                "sub": enc2(tower.fp2_sub(a, b)),
                "mul": enc2(tower.fp2_mul(a, b)),
                "neg_a": enc2(tower.fp2_neg(a)),
                "square_a": enc2(tower.fp2_mul(a, a)),
                "conjugate_a": enc2(tower.fp2_conjugate(a)),
                "norm_a": hex(tower.fp2_norm(a)),
                "a_scaled_by_b0": enc2(tower.fp2_mul(a, (b[0], 0))),
                "a_times_xi": enc2(tower.fp2_mul(a, tower.xi)),
            }
            for a, b in pairs
        ],
    }


def fp6_vectors(tower: Tower, rng: random.Random) -> dict:
    p = tower.p
    zero, one, top = (0, 0), (1, 0), (p - 1, p - 1)
    special = [
        (zero, zero, zero),
        (one, zero, zero),
        (zero, one, zero),  # v
        (zero, zero, one),  # v^2
        (top, top, top),
    ]

    def rand2() -> Fp2:
        return (rng.randrange(p), rng.randrange(p))

    pairs = [(a, b) for a in special for b in special]
    for _ in range(RANDOM_FP6_CASES):
        pairs.append(((rand2(), rand2(), rand2()), (rand2(), rand2(), rand2())))

    return {
        "nonresidue": enc2(tower.xi),
        "ops": [
            {
                "a": enc6(a),
                "b": enc6(b),
                "add": enc6(tower.fp6_add(a, b)),
                "sub": enc6(tower.fp6_sub(a, b)),
                "mul": enc6(tower.fp6_mul(a, b)),
                "neg_a": enc6(tower.fp6_neg(a)),
                "square_a": enc6(tower.fp6_mul(a, a)),
                "a_scaled_by_b0": enc6(tower.fp6_scale(a, b[0])),
                "a_times_v": enc6(tower.fp6_mul_by_v(a)),
            }
            for a, b in pairs
        ],
    }


def fp12_vectors(tower: Tower, rng: random.Random) -> dict:
    p = tower.p
    top2 = (p - 1, p - 1)
    top6 = (top2, top2, top2)
    special = [
        (FP6_ZERO, FP6_ZERO),
        (FP6_ONE, FP6_ZERO),
        (FP6_ZERO, FP6_ONE),  # w
        (top6, top6),
    ]

    def rand6() -> Fp6:
        return tuple((rng.randrange(p), rng.randrange(p)) for _ in range(3))

    pairs = [(a, b) for a in special for b in special]
    for _ in range(RANDOM_FP12_CASES):
        pairs.append(((rand6(), rand6()), (rand6(), rand6())))

    return {
        "ops": [
            {
                "a": enc12(a),
                "b": enc12(b),
                "add": enc12(tower.fp12_add(a, b)),
                "sub": enc12(tower.fp12_sub(a, b)),
                "mul": enc12(tower.fp12_mul(a, b)),
                "neg_a": enc12(tower.fp12_neg(a)),
                "square_a": enc12(tower.fp12_mul(a, a)),
                "conjugate_a": enc12(tower.fp12_conjugate(a)),
                "a_scaled_by_b0": enc12(tower.fp12_scale(a, b[0])),
            }
            for a, b in pairs
        ],
    }


def build() -> dict:
    constants = json.loads(CONSTANTS.read_text())
    p = int(constants["fields"]["Fp"]["modulus"], 16)
    tower = Tower(p, XI)
    return {
        "curve": constants["curve"],
        "seed": SEED,
        "modulus": hex(p),
        "fp2": fp2_vectors(tower, random.Random(f"{SEED}/fp2")),
        "fp6": fp6_vectors(tower, random.Random(f"{SEED}/fp6")),
        "fp12": fp12_vectors(tower, random.Random(f"{SEED}/fp12")),
    }


def main() -> None:
    OUTPUT.write_text(json.dumps(build(), indent=1) + "\n")
    print(f"wrote {OUTPUT}")


if __name__ == "__main__":
    main()
