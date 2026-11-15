"""Check the tower reference model and tests/vectors/bn254_tower_ops.json."""

import json

from hypothesis import given, settings
from hypothesis import strategies as st
from tacit_ref.tower import FP2_ONE, FP2_ZERO, Tower
from tacit_ref.tower_vectors import OUTPUT, XI, build

DATA = json.loads(OUTPUT.read_text())
P = int(DATA["modulus"], 16)
TOWER = Tower(P, XI)

coeff = st.integers(0, P - 1)
fp2 = st.tuples(coeff, coeff)
fp6 = st.tuples(fp2, fp2, fp2)
FP6_ONE = (FP2_ONE, FP2_ZERO, FP2_ZERO)


def test_committed_vectors_are_reproducible():
    assert DATA == build()


def test_u_squared_is_minus_one():
    assert TOWER.fp2_mul((0, 1), (0, 1)) == (P - 1, 0)


def test_xi_is_neither_a_square_nor_a_cube():
    # Fp2* is cyclic of order p^2 - 1. xi is a cube (square) exactly when raising it to
    # (p^2 - 1) / 3 (respectively / 2) gives 1. Both must fail for v^3 - xi, and later
    # w^2 - v, to be irreducible.
    order = P * P - 1
    assert order % 6 == 0
    assert TOWER.fp2_pow(XI, order) == FP2_ONE
    assert TOWER.fp2_pow(XI, order // 3) != FP2_ONE
    assert TOWER.fp2_pow(XI, order // 2) != FP2_ONE


def test_v_cubed_is_xi():
    v = (FP2_ZERO, FP2_ONE, FP2_ZERO)
    assert TOWER.fp6_mul(TOWER.fp6_mul(v, v), v) == (XI, FP2_ZERO, FP2_ZERO)


@settings(max_examples=50, deadline=None)
@given(a=fp2, b=fp2, c=fp2)
def test_fp2_ring_laws(a, b, c):
    mul, add = TOWER.fp2_mul, TOWER.fp2_add
    assert mul(a, b) == mul(b, a)
    assert mul(mul(a, b), c) == mul(a, mul(b, c))
    assert mul(a, add(b, c)) == add(mul(a, b), mul(a, c))
    assert mul(a, TOWER.fp2_conjugate(a)) == (TOWER.fp2_norm(a), 0)


@settings(max_examples=50, deadline=None)
@given(a=fp6, b=fp6, c=fp6)
def test_fp6_ring_laws(a, b, c):
    mul, add = TOWER.fp6_mul, TOWER.fp6_add
    assert mul(a, FP6_ONE) == a
    assert mul(a, b) == mul(b, a)
    assert mul(mul(a, b), c) == mul(a, mul(b, c))
    assert mul(a, add(b, c)) == add(mul(a, b), mul(a, c))
    assert TOWER.fp6_mul_by_v(a) == mul(a, (FP2_ZERO, FP2_ONE, FP2_ZERO))
    assert TOWER.fp6_scale(a, b[0]) == mul(a, (b[0], FP2_ZERO, FP2_ZERO))


def test_vector_values_are_canonical():
    def leaves(node):
        if isinstance(node, str):
            yield node
        elif isinstance(node, list):
            for item in node:
                yield from leaves(item)
        elif isinstance(node, dict):
            for item in node.values():
                yield from leaves(item)

    for level in ("fp2", "fp6"):
        values = list(leaves(DATA[level]))
        assert values
        assert all(0 <= int(v, 16) < P for v in values)
