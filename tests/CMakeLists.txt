add_executable(tacit_tests version_test.cpp prime_field_test.cpp tower_field_test.cpp)
target_link_libraries(tacit_tests PRIVATE
    tacit::tacit GTest::gtest_main nlohmann_json::nlohmann_json)
target_compile_definitions(tacit_tests PRIVATE
    TACIT_EXPECTED_VERSION="${PROJECT_VERSION}"
    TACIT_VECTORS_DIR="${CMAKE_CURRENT_SOURCE_DIR}/vectors")
tacit_configure_target(tacit_tests)
include(GoogleTest)
gtest_discover_tests(tacit_tests)
