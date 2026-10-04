# zk-snark

C++20 zk-SNARK learning project with independent Python/Sage reference models.
Current stage: **P0 infrastructure**. No cryptographic primitives are implemented.

## WSL quick start

Prerequisites: GCC or Clang with C++20 support, CMake >= 3.25, Ninja,
Git and Python >= 3.11 with venv. Initial configuration needs network access
to download pinned GoogleTest and Google Benchmark commits.

```bash
cd ~/code/zk-snark
cmake --preset debug
cmake --build --preset debug --parallel 2
ctest --preset debug
./build/debug/examples/zk_snark_demo
```

## Release and benchmark harness

```bash
cmake --preset release
cmake --build --preset release --parallel 2
ctest --preset release
mkdir -p results
./build/release/benchmarks/zk_snark_bench \
  --benchmark_out=results/harness.json --benchmark_out_format=json
```

This measures harness wiring only, not field arithmetic or proving performance.

## Sanitizers

```bash
cmake --preset asan
cmake --build --preset asan --parallel 2
ctest --preset asan
```

The asan preset enables AddressSanitizer and UndefinedBehaviorSanitizer.
Compiler choice is cached per build directory. Use a fresh directory when
switching compilers, for example `CXX=clang++ cmake --preset debug -B build/clang-debug`.

## Python reference environment

```bash
python3 -m venv .venv
source .venv/bin/activate
python -m pip install -e ".[dev]"
python -c "import zk_snark_ref"
ruff check python
```

If .venv already exists, reuse it. Add independent models in python/zk_snark_ref/
and tests in python/tests/. Run `python -m pytest` after adding the first tests.
Python currently has no native binding; that comes in a later roadmap stage.
Python dependency ranges are not a lockfile; freeze an environment for published
benchmark results. SageMath is a separate runtime, not a pip dependency.

## Layout

```text
include/zk_snark/       Public C++ headers
src/                   C++ implementation
examples/              Executable examples
tests/                 C++ tests and shared vectors
benchmarks/            Google Benchmark cases
python/zk_snark_ref/    Independent Python reference models
python/tests/          Reference and property tests
scripts/               Future Sage parameter generation
cmake/                 Dependencies and target options
docs/                  Roadmap and P0 checklist
.devcontainer/         Debian development image
.github/workflows/     GCC/Clang and Python checks
```

## Devcontainer

Open the project using VS Code in WSL and choose **Reopen in Container**.
Docker must be available in this WSL distribution. The post-create command
prepares Python and performs a debug build and test. It does not install Sage.

## Next step

Implement the Sage BN254 parameter generator described in scripts/README.md,
then begin the finite-field module. See docs/p0-checklist.md and the supplied
docs/roadmap.md. The roadmap is retained as planning input; ecosystem claims
and implementation choices still need verification as work progresses.

No license has been selected yet; choose one before a public release.
