#include <tacit/version.hpp>

#include <gtest/gtest.h>

// Wiring check only: confirms the executable links and calls the project library.
TEST(BuildSmoke, ExposesConfiguredVersion) {
    EXPECT_EQ(tacit::version(), TACIT_EXPECTED_VERSION);
}
