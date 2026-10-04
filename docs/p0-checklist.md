# P0 checklist

- [x] C++20 static library, example and CMake presets.
- [x] GoogleTest and Google Benchmark integration.
- [x] Python package and optional development dependencies.
- [x] GCC/Clang CI workflow and devcontainer configuration.
- [ ] Verify the workflow on GitHub after publishing the repository.
- [ ] Build and open the devcontainer.
- [ ] Install SageMath and implement the constant generator.
- [ ] Independently verify BN254 Fp/Fr parameters and commit test vectors.

Finite-field arithmetic, curves, pairings, NTT/MSM and Groth16 are intentionally
left for the roadmap stages. The benchmark currently checks harness wiring only.

Before publishing, choose an open-source license with both contributors.
No license has been selected automatically.
