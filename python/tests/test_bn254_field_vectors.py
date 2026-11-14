"""Check tests/vectors/bn254_field_ops.json against the generator and the Montgomery model."""

import json

import pytest
from tacit_ref.field_vectors import OUTPUT, build
from tacit_ref.montgomery import Montgomery

DATA = json.loads(OUTPUT.read_text())
FIELDS = ("Fp", "Fr")


def test_committed_vectors_are_reproducible():
    assert DATA == build()


@pytest.mark.parametrize("name", FIELDS)
def test_ops_cover_edges_and_stay_canonical(name):
    field = DATA["fields"][name]
    p = int(field["modulus"], 16)
    inputs = {(int(c["a"], 16), int(c["b"], 16)) for c in field["ops"]}

    assert {(0, 0), (p - 1, p - 1), (p - 1, 1), (0, p - 1)} <= inputs
    for case in field["ops"]:
        assert all(0 <= int(v, 16) < p for v in case.values())


@pytest.mark.parametrize("name", FIELDS)
def test_mul_and_square_match_montgomery_model(name):
    field = DATA["fields"][name]
    mont = Montgomery(int(field["modulus"], 16))

    for case in field["ops"]:
        a = mont.to_mont(int(case["a"], 16))
        b = mont.to_mont(int(case["b"], 16))
        assert mont.from_mont(mont.mul(a, b)) == int(case["mul"], 16)
        assert mont.from_mont(mont.mul(a, a)) == int(case["square_a"], 16)


@pytest.mark.parametrize("name", FIELDS)
def test_reduce_inputs_are_not_canonical(name):
    field = DATA["fields"][name]
    p = int(field["modulus"], 16)

    for case in field["reduce"]:
        value = int(case["value"], 16)
        assert p <= value < 1 << 256
        assert int(case["reduced"], 16) == value % p
