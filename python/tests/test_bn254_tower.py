"""Check the tower reference model and tests/vectors/bn254_tower_ops.json."""

import json

from hypothesis import assume, given, settings
from hypothesis import strategies as st
from tacit_ref.tower import FP2_ONE, FP2_ZERO, FP6_ONE, FP6_ZERO, Tower
from tacit_ref.tower_vectors import OUTPUT, XI, build

DATA = json.loads(OUTPUT.read_text())
P = int(DATA["modulus"], 16)
TOWER = Tower(P, XI)

coeff = st.integers(0, P - 1)
fp2 = st.tuples(coeff, coeff)
fp6 = st.tuples(fp2, fp2, fp2)
fp12 = st.tuples(fp6, fp6)
V = (FP2_ZERO, FP2_ONE, FP2_ZERO)
FP12_ONE = (FP6_ONE, FP6_ZERO)


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
    assert TOWER.fp6_mul(TOWER.fp6_mul(V, V), V) == (XI, FP2_ZERO, FP2_ZERO)


def test_v_is_not_a_square_in_fp6():
    # Fp6* is cyclic of order p^6 - 1, so v is a square exactly when v^((p^6 - 1) / 2) = 1.
    order = P**6 - 1
    assert TOWER.fp6_pow(V, order) == FP6_ONE
    assert TOWER.fp6_pow(V, order // 2) != FP6_ONE


def test_w_squared_is_v():
    w = (FP6_ZERO, FP6_ONE)
    assert TOWER.fp12_mul(w, w) == (V, FP6_ZERO)


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
    assert TOWER.fp6_mul_by_v(a) == mul(a, V)
    assert TOWER.fp6_scale(a, b[0]) == mul(a, (b[0], FP2_ZERO, FP2_ZERO))


@settings(max_examples=25, deadline=None)
@given(a=fp12, b=fp12, c=fp12)
def test_fp12_ring_laws(a, b, c):
    mul, add = TOWER.fp12_mul, TOWER.fp12_add
    assert mul(a, FP12_ONE) == a
    assert mul(a, b) == mul(b, a)
    assert mul(mul(a, b), c) == mul(a, mul(b, c))
    assert mul(a, add(b, c)) == add(mul(a, b), mul(a, c))
    assert TOWER.fp12_scale(a, b[0]) == mul(a, (b[0], FP6_ZERO))
    # a * conjugate(a) lies in Fp6: its w coefficient vanishes.
    assert mul(a, TOWER.fp12_conjugate(a))[1] == FP6_ZERO


@settings(max_examples=50, deadline=None)
@given(a=fp2, b=fp6, c=fp12)
def test_inverses_multiply_back_to_one(a, b, c):
    assume(a != FP2_ZERO and b != FP6_ZERO and c != (FP6_ZERO, FP6_ZERO))
    assert TOWER.fp2_mul(a, TOWER.fp2_inv(a)) == FP2_ONE
    assert TOWER.fp6_mul(b, TOWER.fp6_inv(b)) == FP6_ONE
    assert TOWER.fp12_mul(c, TOWER.fp12_inv(c)) == FP12_ONE


def test_zero_has_no_inverse():
    assert TOWER.fp2_inv(FP2_ZERO) is None
    assert TOWER.fp6_inv(FP6_ZERO) is None
    assert TOWER.fp12_inv((FP6_ZERO, FP6_ZERO)) is None


def test_vector_inverses_are_null_only_for_zero():
    for level, zero in (("fp2", ["0x0"] * 2), ("fp6", [["0x0"] * 2] * 3)):
        for case in DATA[level]["ops"]:
            assert (case["inverse_a"] is None) == (case["a"] == zero)
    zero12 = [[["0x0"] * 2] * 3] * 2
    for case in DATA["fp12"]["ops"]:
        assert (case["inverse_a"] is None) == (case["a"] == zero12)


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

    for level in ("fp2", "fp6", "fp12"):
        values = list(leaves(DATA[level]))
        assert values
        assert all(0 <= int(v, 16) < P for v in values)
