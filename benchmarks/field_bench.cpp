#include <tacit/bn254/fields.hpp>

#include <benchmark/benchmark.h>

#include <utility>

// Portable C++ field arithmetic on BN254. Each loop feeds its result into the next
// iteration, so the reported time is the latency of one operation.

namespace {

using tacit::u64;
using tacit::bn254::Fp;
using tacit::bn254::Fp2;
using tacit::bn254::Fp6;
using tacit::bn254::Fr;

// A fixed, non-trivial element of each field, selected by a small seed.
template <typename Field> struct Sample {
  static auto make(u64 seed) -> Field {
    Field x = Field::from_u64(0x9e3779b97f4a7c15 + seed);
    for (int i = 0; i < 8; ++i) {
      x = x.square() + Field::from_u64(0xc2b2ae3d27d4eb4f);
    }
    return x;
  }
};

template <> struct Sample<Fp2> {
  static auto make(u64 seed) -> Fp2 {
    return {Sample<Fp>::make(2 * seed), Sample<Fp>::make(2 * seed + 1)};
  }
};

template <> struct Sample<Fp6> {
  static auto make(u64 seed) -> Fp6 {
    return {Sample<Fp2>::make(3 * seed), Sample<Fp2>::make(3 * seed + 1),
            Sample<Fp2>::make(3 * seed + 2)};
  }
};

template <typename Field> auto operands() -> std::pair<Field, Field> {
  return {Sample<Field>::make(1), Sample<Field>::make(100)};
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

BENCHMARK(BM_Add<Fp>)->Name("bn254/Fp/add");
BENCHMARK(BM_Sub<Fp>)->Name("bn254/Fp/sub");
BENCHMARK(BM_Mul<Fp>)->Name("bn254/Fp/mul");
BENCHMARK(BM_Square<Fp>)->Name("bn254/Fp/square");
BENCHMARK(BM_Add<Fr>)->Name("bn254/Fr/add");
BENCHMARK(BM_Sub<Fr>)->Name("bn254/Fr/sub");
BENCHMARK(BM_Mul<Fr>)->Name("bn254/Fr/mul");
BENCHMARK(BM_Square<Fr>)->Name("bn254/Fr/square");
BENCHMARK(BM_Add<Fp2>)->Name("bn254/Fp2/add");
BENCHMARK(BM_Mul<Fp2>)->Name("bn254/Fp2/mul");
BENCHMARK(BM_Square<Fp2>)->Name("bn254/Fp2/square");
BENCHMARK(BM_Add<Fp6>)->Name("bn254/Fp6/add");
BENCHMARK(BM_Mul<Fp6>)->Name("bn254/Fp6/mul");
BENCHMARK(BM_Square<Fp6>)->Name("bn254/Fp6/square");

} // namespace
