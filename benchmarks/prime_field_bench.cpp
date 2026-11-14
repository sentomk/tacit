#include <tacit/bn254/fields.hpp>

#include <benchmark/benchmark.h>

// Portable C++ prime field arithmetic on BN254. Each loop feeds its result into the next
// iteration, so the reported time is the latency of one operation.

namespace {

template <typename Field> auto operands() -> std::pair<Field, Field> {
  Field x = Field::from_u64(0x9e3779b97f4a7c15);
  Field y = Field::from_u64(0xc2b2ae3d27d4eb4f);
  for (int i = 0; i < 8; ++i) {
    x = x.square() + y;
    y = y * x - Field::one();
  }
  return {x, y};
}

template <typename Field> void BM_Add(benchmark::State &state) {
  auto [x, y] = operands<Field>();
  benchmark::DoNotOptimize(y);
  for (auto _ : state) {
    x += y;
    benchmark::DoNotOptimize(x);
  }
}

template <typename Field> void BM_Sub(benchmark::State &state) {
  auto [x, y] = operands<Field>();
  benchmark::DoNotOptimize(y);
  for (auto _ : state) {
    x -= y;
    benchmark::DoNotOptimize(x);
  }
}

template <typename Field> void BM_Mul(benchmark::State &state) {
  auto [x, y] = operands<Field>();
  benchmark::DoNotOptimize(y);
  for (auto _ : state) {
    x *= y;
    benchmark::DoNotOptimize(x);
  }
}

template <typename Field> void BM_Square(benchmark::State &state) {
  auto [x, y] = operands<Field>();
  for (auto _ : state) {
    x = x.square();
    benchmark::DoNotOptimize(x);
  }
}

using tacit::bn254::Fp;
using tacit::bn254::Fr;

BENCHMARK(BM_Add<Fp>)->Name("bn254/Fp/add");
BENCHMARK(BM_Sub<Fp>)->Name("bn254/Fp/sub");
BENCHMARK(BM_Mul<Fp>)->Name("bn254/Fp/mul");
BENCHMARK(BM_Square<Fp>)->Name("bn254/Fp/square");
BENCHMARK(BM_Add<Fr>)->Name("bn254/Fr/add");
BENCHMARK(BM_Sub<Fr>)->Name("bn254/Fr/sub");
BENCHMARK(BM_Mul<Fr>)->Name("bn254/Fr/mul");
BENCHMARK(BM_Square<Fr>)->Name("bn254/Fr/square");

} // namespace
