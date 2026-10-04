add_executable(zk_snark_tests version_test.cpp)
target_link_libraries(zk_snark_tests PRIVATE zk_snark::zk_snark GTest::gtest_main)
target_compile_definitions(zk_snark_tests PRIVATE
    ZK_SNARK_EXPECTED_VERSION="${PROJECT_VERSION}")
zk_snark_configure_target(zk_snark_tests)
include(GoogleTest)
gtest_discover_tests(zk_snark_tests)
