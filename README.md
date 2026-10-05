# tacit

tacit is a C++20 library that implements zk-SNARKs from scratch: finite fields,
elliptic curves and pairings, NTT and MSM, and the Groth16 proof system. On top
of the library we plan a proof-of-reserves application for exchanges.

## What is it?

- A core library in modern C++ with compile-time parameterized fields, targeting
  BN254 (usable with Ethereum precompiles) and BLS12-381.
- Fast paths for the hot loops: x86-64 assembly and CUDA.
- A Groth16 prover and verifier that can consume Circom R1CS files and produce
  proofs compatible with snarkjs.
- Independent Python and Sage reference models with shared test vectors, which the
  C++ implementation is checked against.
- A proof-of-reserves system built on the library: an exchange proves that its
  liabilities do not exceed its assets without publishing individual balances,
  and each user can check that their own balance is included.

## Why we're building it

- We want to understand zk-SNARKs by building the whole stack from the bottom up,
  not by treating it as a black box.
- We care about correctness first. Every component gets an independent reference
  model and test vectors, so the C++ code is checked against something other than
  itself.
- We want it fast, and we want to prove that with numbers: compile-time
  parameters, hand-tuned fast paths, and benchmarks against existing libraries.
- We want a real application to keep the library honest. Proof of reserves needs
  everything, from field arithmetic to on-chain verification.
