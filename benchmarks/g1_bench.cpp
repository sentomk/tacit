#include <tacit/bn254/g1.hpp>

#include <benchmark/benchmark.h>

#include <array>

// Portable C++ G1 arithmetic on BN254. As in the field benchmarks, each loop feeds its result
// into the next iteration, so the reported time is the latency of one operation.

namespace {

using tacit::u64;
using tacit::bn254::Fr;
using tacit::bn254::G1;
using tacit::bn254::G1Affine;

// A fixed, full-size scalar and two unrelated points.
auto scalar() -> Fr {
  Fr k = Fr::from_u64(0x9e3779b97f4a7c15);
  for (int i = 0; i < 8; ++i) {
    k = k.square() + Fr::from_u64(0xc2b2ae3d27d4eb4f);
  }
  return k;
}

auto point(u64 seed) -> G1 { return G1::generator().mul(std::array<u64, 1>{seed}); }

void BM_G1Add(benchmark::State &state) {
  G1 p = point(0x1234567);
  const G1 q = point(0x7654321);
  for (auto _ : state) {
    p += q;
    benchmark::DoNotOptimize(p);
  }
}

void BM_G1AddMixed(benchmark::State &state) {
  G1 p = point(0x1234567);
  const G1Affine q = point(0x7654321).to_affine();
  for (auto _ : state) {
    p += q;
    benchmark::DoNotOptimize(p);
  }
}

void BM_G1Double(benchmark::State &state) {
  G1 p = point(0x1234567);
  for (auto _ : state) {
    p = p.doubled();
    benchmark::DoNotOptimize(p);
  }
}

void BM_G1ToAffine(benchmark::State &state) {
  const G1 p = point(0x1234567);
  for (auto _ : state) {
    benchmark::DoNotOptimize(p.to_affine());
  }
}

void BM_G1ScalarMul(benchmark::State &state) {
  G1 p = point(0x1234567);
  const Fr k = scalar();
  for (auto _ : state) {
    p = p * k;
    benchmark::DoNotOptimize(p);
  }
}

BENCHMARK(BM_G1Add)->Name("bn254/G1/add");
BENCHMARK(BM_G1AddMixed)->Name("bn254/G1/add_mixed");
BENCHMARK(BM_G1Double)->Name("bn254/G1/double");
BENCHMARK(BM_G1ToAffine)->Name("bn254/G1/to_affine");
BENCHMARK(BM_G1ScalarMul)->Name("bn254/G1/scalar_mul");

} // namespace
