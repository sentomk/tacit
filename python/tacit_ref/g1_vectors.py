"""Generate tests/vectors/bn254_g1_ops.json: test vectors for the BN254 group G1.

Expected values come from the affine arithmetic in tacit_ref.curve. The moduli are read from
tests/vectors/bn254_constants.json. The output is deterministic, so running this again
reproduces the committed file byte for byte:

    python -m tacit_ref.g1_vectors
"""

import json
import random
from pathlib import Path

from tacit_ref.curve import Curve, Point

VECTORS_DIR = Path(__file__).resolve().parents[2] / "tests" / "vectors"
CONSTANTS = VECTORS_DIR / "bn254_constants.json"
OUTPUT = VECTORS_DIR / "bn254_g1_ops.json"

SEED = "tacit/bn254/g1-ops/v1"
B = 3
GENERATOR = (1, 2)
RANDOM_POINTS = 3
RANDOM_SCALARS = 4


def enc(pt: Point) -> list[str] | None:
    return None if pt is None else [hex(pt[0]), hex(pt[1])]


def curve_and_order() -> tuple[Curve, int]:
    constants = json.loads(CONSTANTS.read_text())
    p = int(constants["fields"]["Fp"]["modulus"], 16)
    r = int(constants["fields"]["Fr"]["modulus"], 16)
    return Curve(p, B), r


def build() -> dict:
    curve, r = curve_and_order()
    rng = random.Random(SEED)
    g = GENERATOR

    random_points = [curve.mul(g, rng.randrange(1, r)) for _ in range(RANDOM_POINTS)]
    p1 = random_points[0]
    points = [None, g, curve.neg(g), curve.mul(g, 2), *random_points, curve.neg(p1)]

    top = (1 << 256) - 1
    scalars = [0, 1, 2, 3, r - 1, r, r + 1, 2 * r, top]
    scalars += [rng.randrange(r) for _ in range(RANDOM_SCALARS)]
    bases = [g, p1, None]

    # Points that satisfy no curve equation of the right form.
    off_curve = [(0, 0), (1, 3), (p1[0], (p1[1] + 1) % curve.p), (0, 3)]

    return {
        "curve": "bn254",
        "seed": SEED,
        "b": hex(B),
        "order": hex(r),
        "generator": enc(g),
        "add": [
            {"a": enc(a), "b": enc(b), "sum": enc(curve.add(a, b))} for a in points for b in points
        ],
        "double": [{"a": enc(a), "doubled": enc(curve.add(a, a))} for a in points],
        "mul": [
            {"base": enc(base), "scalar": hex(k), "product": enc(curve.mul(base, k))}
            for base in bases
            for k in scalars
        ],
        "off_curve": [enc(pt) for pt in off_curve],
    }


def main() -> None:
    OUTPUT.write_text(json.dumps(build(), indent=1) + "\n")
    print(f"wrote {OUTPUT}")


if __name__ == "__main__":
    main()
