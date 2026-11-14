"""Generate tests/vectors/bn254_field_ops.json: prime field test vectors for Fp and Fr.

Every expected value is computed with plain Python integers. The moduli are read from
tests/vectors/bn254_constants.json. The output is deterministic, so running this again
reproduces the committed file byte for byte:

    python -m tacit_ref.field_vectors
"""

import json
import random
from pathlib import Path

VECTORS_DIR = Path(__file__).resolve().parents[2] / "tests" / "vectors"
CONSTANTS = VECTORS_DIR / "bn254_constants.json"
OUTPUT = VECTORS_DIR / "bn254_field_ops.json"

SEED = "tacit/bn254/field-ops/v1"
RANDOM_CASES = 64
RANDOM_REDUCE_CASES = 8


def field_vectors(name: str, p: int) -> dict:
    rng = random.Random(f"{SEED}/{name}")
    limb_bits = 64 * ((p.bit_length() + 63) // 64)

    edges = (0, 1, 2, p - 2, p - 1)
    pairs = [(a, b) for a in edges for b in edges]
    pairs += [(rng.randrange(p), rng.randrange(p)) for _ in range(RANDOM_CASES)]

    # Inputs at or above the modulus, up to the largest value the limbs can hold.
    wide = [p, p + 1, 2 * p - 1, 2 * p, (1 << limb_bits) - 1]
    wide += [rng.randrange(p, 1 << limb_bits) for _ in range(RANDOM_REDUCE_CASES)]

    return {
        "modulus": hex(p),
        "ops": [
            {
                "a": hex(a),
                "b": hex(b),
                "add": hex((a + b) % p),
                "sub": hex((a - b) % p),
                "mul": hex(a * b % p),
                "neg_a": hex(-a % p),
                "square_a": hex(a * a % p),
            }
            for a, b in pairs
        ],
        "reduce": [{"value": hex(v), "reduced": hex(v % p)} for v in wide],
    }


def build() -> dict:
    constants = json.loads(CONSTANTS.read_text())
    return {
        "curve": constants["curve"],
        "seed": SEED,
        "fields": {
            name: field_vectors(name, int(field["modulus"], 16))
            for name, field in constants["fields"].items()
        },
    }


def main() -> None:
    OUTPUT.write_text(json.dumps(build(), indent=2) + "\n")
    print(f"wrote {OUTPUT}")


if __name__ == "__main__":
    main()
