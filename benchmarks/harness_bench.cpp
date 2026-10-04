#include <zk_snark/version.hpp>

#include <benchmark/benchmark.h>

// Harness check only; this is NOT a cryptographic performance measurement.
static void BM_HarnessVersion(benchmark::State& state) {
    for (auto _ : state) {
        benchmark::DoNotOptimize(zk_snark::version());
    }
}

BENCHMARK(BM_HarnessVersion);
