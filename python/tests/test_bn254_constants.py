"""Cross-check tests/vectors/bn254_constants.json with independent Python arithmetic.

The vectors come from scripts/generate_constants.sage. Nothing here calls Sage: every
value is recomputed from the modulus with plain integers, and the moduli themselves are
compared with the constants quoted in the Ethereum EIPs.
"""

import json
from pathlib import Path

import pytest
from hypothesis import given, settings
from hypothesis import strategies as st
from tacit_ref.montgomery import LIMB_BITS, WORD, Montgomery, limb_count
from tacit_ref.numtheory import is_probable_prime, smallest_non_residue, two_adicity

VECTORS = Path(__file__).resolve().parents[2] / "tests" / "vectors" / "bn254_constants.json"
DATA = json.loads(VECTORS.read_text())
FIELDS = ("Fp", "Fr")

# Quoted from EIP-196 (field modulus p) and EIP-197 (group order q), not from the vectors.
EIP_P = 21888242871839275222246405745257275088696311157297823662689037894645226208583
EIP_Q = 21888242871839275222246405745257275088548364400416034343698204186575808495617
X = 4965661367192848881  # BN254 curve parameter


def modulus(name: str) -> int:
    return int(DATA["fields"][name]["modulus"], 16)


def test_curve_parameter_generates_both_moduli():
    assert 36 * X**4 + 36 * X**3 + 24 * X**2 + 6 * X + 1 == EIP_P
    assert 36 * X**4 + 36 * X**3 + 18 * X**2 + 6 * X + 1 == EIP_Q


def test_header():
    assert DATA["curve"] == "bn254"
    assert DATA["limb_bits"] == LIMB_BITS
    assert int(DATA["curve_param_x"], 16) == X
    assert set(DATA["fields"]) == set(FIELDS)


def test_moduli_match_eips():
    assert modulus("Fp") == EIP_P
    assert modulus("Fr") == EIP_Q


@pytest.mark.parametrize("name", FIELDS)
def test_modulus_is_prime(name):
    assert is_probable_prime(modulus(name))


@pytest.mark.parametrize("name", FIELDS)
def test_limb_layout(name):
    field = DATA["fields"][name]
    p = modulus(name)
    limbs = [int(v, 16) for v in field["modulus_limbs"]]

    assert field["bits"] == p.bit_length()
    assert field["limbs"] == limb_count(p) == len(limbs)
    assert all(0 <= v < WORD for v in limbs)
    assert limbs[-1] != 0
    assert sum(v << (LIMB_BITS * i) for i, v in enumerate(limbs)) == p


@pytest.mark.parametrize("name", FIELDS)
def test_montgomery_constants(name):
    field = DATA["fields"][name]
    p = modulus(name)
    mont = Montgomery(p)

    assert int(field["r_mod_p"], 16) == mont.r == (1 << (LIMB_BITS * field["limbs"])) % p
    assert int(field["r2_mod_p"], 16) == mont.r2
    n0 = int(field["n0"], 16)
    assert n0 == mont.n0
    assert (n0 * p + 1) % WORD == 0


@pytest.mark.parametrize("name", FIELDS)
def test_montgomery_edge_values(name):
    p = modulus(name)
    mont = Montgomery(p)
    edges = (0, 1, 2, p - 2, p - 1)

    assert mont.to_mont(1) == mont.r
    assert mont.from_mont(mont.r) == 1
    for a in edges:
        assert mont.from_mont(mont.to_mont(a)) == a
        for b in edges:
            assert mont.from_mont(mont.mul(mont.to_mont(a), mont.to_mont(b))) == a * b % p


@pytest.mark.parametrize("name", FIELDS)
@settings(max_examples=200, deadline=None)
@given(data=st.data())
def test_montgomery_mul_matches_modular_mul(name, data):
    p = modulus(name)
    mont = Montgomery(p)
    a = data.draw(st.integers(0, p - 1))
    b = data.draw(st.integers(0, p - 1))

    product = mont.mul(mont.to_mont(a), mont.to_mont(b))
    assert 0 <= product < p
    assert mont.from_mont(product) == a * b % p


@pytest.mark.parametrize("name", FIELDS)
def test_two_adicity(name):
    field = DATA["fields"][name]
    p = modulus(name)
    s, t = two_adicity(p)

    assert field["two_adicity"] == s
    assert int(field["odd_part"], 16) == t
    assert t % 2 == 1
    assert t << s == p - 1


@pytest.mark.parametrize("name", FIELDS)
def test_nonresidue_is_the_smallest(name):
    assert DATA["fields"][name]["nonresidue"] == smallest_non_residue(modulus(name))


@pytest.mark.parametrize("name", FIELDS)
def test_two_adic_roots_of_unity(name):
    field = DATA["fields"][name]
    p = modulus(name)
    s, t = two_adicity(p)
    roots = [int(v, 16) for v in field["two_adic_roots"]]

    assert len(roots) == s + 1
    assert roots[0] == 1
    assert int(field["root_of_unity"], 16) == roots[-1] == pow(field["nonresidue"], t, p)
    for k, w in enumerate(roots):
        assert 0 <= w < p
        assert pow(w, 1 << k, p) == 1
        if k > 0:
            assert pow(w, 1 << (k - 1), p) == p - 1  # so the order is exactly 2^k
    for k in range(1, s + 1):
        assert roots[k] * roots[k] % p == roots[k - 1]
