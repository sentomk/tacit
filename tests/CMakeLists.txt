add_executable(tacit_tests version_test.cpp)
target_link_libraries(tacit_tests PRIVATE tacit::tacit GTest::gtest_main)
target_compile_definitions(tacit_tests PRIVATE
    TACIT_EXPECTED_VERSION="${PROJECT_VERSION}")
tacit_configure_target(tacit_tests)
include(GoogleTest)
gtest_discover_tests(tacit_tests)
