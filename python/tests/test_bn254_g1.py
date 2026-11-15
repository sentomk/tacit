"""Check the curve reference model and the G1 vectors, including the EIP-196 ones."""

import json

from hypothesis import given, settings
from hypothesis import strategies as st
from tacit_ref.eip196_vectors import OUTPUT as EIP196
from tacit_ref.g1_vectors import GENERATOR, OUTPUT, build, curve_and_order

DATA = json.loads(OUTPUT.read_text())
EIP = json.loads(EIP196.read_text())
CURVE, R = curve_and_order()
G = GENERATOR

scalar = st.integers(0, R - 1)


def dec(pt):
    return None if pt is None else (int(pt[0], 16), int(pt[1], 16))


def test_committed_vectors_are_reproducible():
    assert DATA == build()


def test_generator_has_order_r():
    assert CURVE.is_on_curve(G)
    assert CURVE.mul(G, R) is None
    assert CURVE.mul(G, R - 1) == CURVE.neg(G)


@settings(max_examples=20, deadline=None)
@given(a=scalar, b=scalar)
def test_scalar_multiplication_is_a_homomorphism(a, b):
    assert CURVE.add(CURVE.mul(G, a), CURVE.mul(G, b)) == CURVE.mul(G, (a + b) % R)
    assert CURVE.mul(CURVE.mul(G, a), b) == CURVE.mul(G, a * b % R)


def test_vector_points_are_on_curve():
    for case in DATA["add"]:
        assert all(CURVE.is_on_curve(dec(case[k])) for k in ("a", "b", "sum"))
    for case in DATA["mul"]:
        assert CURVE.is_on_curve(dec(case["product"]))
    for pt in DATA["off_curve"]:
        assert not CURVE.is_on_curve(dec(pt))


def test_add_cases_cover_the_special_cases():
    pairs = {(case["a"] is None, case["b"] is None, case["sum"] is None) for case in DATA["add"]}
    assert (True, True, True) in pairs  # infinity + infinity
    assert (False, False, True) in pairs  # P + (-P)
    assert any(case["a"] == case["b"] and case["a"] is not None for case in DATA["add"])


def test_eip196_vectors_match_the_reference_model():
    assert EIP["add"] and EIP["mul"]
    for case in EIP["add"]:
        a, b = dec(case["a"]), dec(case["b"])
        assert CURVE.is_on_curve(a) and CURVE.is_on_curve(b), case["name"]
        assert CURVE.add(a, b) == dec(case["sum"]), case["name"]
    for case in EIP["mul"]:
        base = dec(case["base"])
        assert CURVE.is_on_curve(base), case["name"]
        assert CURVE.mul(base, int(case["scalar"], 16)) == dec(case["product"]), case["name"]
