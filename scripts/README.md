# Parameter generation (next P0 task)

Implement `generate_constants.sage` here as a separate learning step.

Start with Ethereum BN254 and explicitly distinguish:
- Fp: base field for curve coordinates.
- Fr: scalar field / circuit field.

For each modulus, record the source and derive:
- 64-bit limb layout and limb count.
- Montgomery R = 2^(64 * limb_count) mod p, R^2 mod p,
  and n0 = -p^(-1) mod 2^64.
- Two-adicity and a root of exact order 2^s for the selected field.

Use Sage to verify primality, modular identities and exact root order.
Write reproducible JSON into tests/vectors/ and later consume the same vectors
from Python and C++ tests. Do not manually copy constants between implementations.

SageMath is not installed by the scaffold. Run the future script with
`sage scripts/generate_constants.sage` in a separately prepared Sage environment.
