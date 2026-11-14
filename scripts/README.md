# Parameter generation

`generate_constants.sage` derives the field constants for Ethereum BN254 and writes
them to `tests/vectors/bn254_constants.json`.

It covers two fields and keeps them explicitly apart:
- Fp: base field for curve coordinates.
- Fr: scalar field / circuit field.

For each modulus it records the source and derives:
- 64-bit limb layout and limb count.
- Montgomery R = 2^(64 * limb_count) mod p, R^2 mod p,
  and n0 = -p^(-1) mod 2^64.
- Two-adicity and a root of exact order 2^s.

Sage verifies primality, the modular identities and the exact root order. The JSON
output is reproducible and is the single source for the Python and C++ tests. Do not
manually copy constants between implementations.

SageMath is not part of the pip dependencies. It is pinned in `environment.yml`
and installed from conda-forge with micromamba:

```bash
micromamba create -f environment.yml
micromamba run -n sage sage scripts/generate_constants.sage
```

Generated vectors are committed under tests/vectors/, so building, testing and
CI do not need Sage.

The field arithmetic vectors in `tests/vectors/bn254_field_ops.json` do not need Sage.
They are produced from the constants above by the Python reference package:

```bash
python -m tacit_ref.field_vectors
```

The devcontainer ships micromamba but not Sage. Run the first command above once
inside it; the environment is kept in the `tacit-micromamba` Docker volume, so it
survives container rebuilds.
