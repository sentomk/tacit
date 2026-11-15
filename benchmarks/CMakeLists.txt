add_executable(tacit_bench field_bench.cpp)
target_link_libraries(tacit_bench PRIVATE
    tacit::tacit benchmark::benchmark_main)
tacit_configure_target(tacit_bench)
