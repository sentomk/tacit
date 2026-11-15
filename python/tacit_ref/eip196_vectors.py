"""Convert go-ethereum's EIP-196 precompile tests into tests/vectors/bn254_g1_eip196.json.

The source files are core/vm/testdata/precompiles/bn256Add.json and bn256ScalarMul.json in
go-ethereum, at the commit recorded in SOURCE. They hold the raw precompile inputs and
outputs; this script decodes them into points and scalars. It reads local copies, so it
needs no network access:

    python -m tacit_ref.eip196_vectors path/to/bn256Add.json path/to/bn256ScalarMul.json

EIP-196 encodes a point as 32-byte big-endian x and y, with (0, 0) for the point at infinity.
Inputs shorter than the expected length are padded with zero bytes and longer ones are
truncated, as the precompiles do.
"""

import json
import sys
from pathlib import Path

OUTPUT = Path(__file__).resolve().parents[2] / "tests" / "vectors" / "bn254_g1_eip196.json"

SOURCE = (
    "https://github.com/ethereum/go-ethereum/tree/"
    "8059ff8beb3170eb8b977169ac51bb0423c97ee1/core/vm/testdata/precompiles"
)
WORD = 64  # hex digits per 32-byte word


def words(hex_input: str, count: int) -> list[int]:
    padded = hex_input.ljust(count * WORD, "0")[: count * WORD]
    return [int(padded[i * WORD : (i + 1) * WORD], 16) for i in range(count)]


def point(x: int, y: int) -> list[str] | None:
    return None if x == 0 and y == 0 else [hex(x), hex(y)]


def convert(add_cases: list[dict], mul_cases: list[dict]) -> dict:
    add = []
    for case in add_cases:
        x1, y1, x2, y2 = words(case["Input"], 4)
        x3, y3 = words(case["Expected"], 2)
        add.append(
            {"name": case["Name"], "a": point(x1, y1), "b": point(x2, y2), "sum": point(x3, y3)}
        )

    mul = []
    for case in mul_cases:
        x, y, k = words(case["Input"], 3)
        x3, y3 = words(case["Expected"], 2)
        mul.append(
            {"name": case["Name"], "base": point(x, y), "scalar": hex(k), "product": point(x3, y3)}
        )

    return {"source": SOURCE, "add": add, "mul": mul}


def main() -> None:
    add_path, mul_path = (Path(arg) for arg in sys.argv[1:3])
    data = convert(json.loads(add_path.read_text()), json.loads(mul_path.read_text()))
    OUTPUT.write_text(json.dumps(data, indent=1) + "\n")
    print(f"wrote {OUTPUT}")


if __name__ == "__main__":
    main()
