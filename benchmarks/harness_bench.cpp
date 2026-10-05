#include <tacit/version.hpp>

#include <benchmark/benchmark.h>

// Harness check only; this is NOT a cryptographic performance measurement.
static void BM_HarnessVersion(benchmark::State& state) {
    for (auto _ : state) {
        benchmark::DoNotOptimize(tacit::version());
    }
}

BENCHMARK(BM_HarnessVersion);
