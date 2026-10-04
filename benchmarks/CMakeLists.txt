add_executable(zk_snark_bench harness_bench.cpp)
target_link_libraries(zk_snark_bench PRIVATE
    zk_snark::zk_snark benchmark::benchmark_main)
zk_snark_configure_target(zk_snark_bench)
